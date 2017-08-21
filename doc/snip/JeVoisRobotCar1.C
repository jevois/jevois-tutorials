// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// JeVois Smart Embedded Machine Vision Toolkit - Copyright (C) 2017 by Laurent Itti, the University of Southern
// California (USC), and iLab at USC. See http://iLab.usc.edu and http://jevois.org for information about this project.
//
// This file is part of the JeVois Smart Embedded Machine Vision Toolkit.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software
// Foundation, version 2.  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.  You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
// Contact information: Laurent Itti - 3641 Watt Way, HNB-07A - Los Angeles, CA 90089-2520 - USA.
// Tel: +1 213 740 3527 - itti@pollux.usc.edu - http://iLab.usc.edu - http://jevois.org
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// You need to install the Arduino PID Library by Brett Beauregard
#include <PID_v1.h>
#include <Servo.h>

// Pins setup: Use RXLED(17) on Arduino pro micro 32u4 which does not have LED(13)
#define LPWMPIN 3
#define LIN1PIN 2
#define LIN2PIN 21
#define RPWMPIN 5
#define RIN1PIN 20
#define RIN2PIN 4
#define LEDPIN 17
#define PANPIN 9
#define TILTPIN 10

// Serial port to use for JeVois:
// On chips with USB (e.g., 32u4), that usually is Serial1. On chips without USB, use Serial.
#define SERIAL Serial1

// Serial port for debugging (optional):
#define DBGSERIAL Serial

// Buffer for received serial port bytes:
#define INLEN 128
char instr[INLEN + 1];

// Gain values to compensate for unbalanced motors: Do not exceed 655. Give a lower value to the motor that is faster.
// E.g., if your car vires to the left when both motors are 100% forward, your right motor is a bit faster, so decrease
// rightgain to compensate.
long leftgain = 655;
long rightgain = 655;

// Desired target width in standardized JeVois coordinates (a value of 2000 would occupy the whole field of view):
int TARGW = 300;

// Create a PID for target width, and one for steering angle:
double wset, win, wout, aset, ain, aout;
PID wpid(&win, &wout, &wset, 1.0, 0.01, 0.01, DIRECT);
PID apid(&ain, &aout, &aset, 0.5, 0.05, 0.02, DIRECT);

// current speed and steering:
double speed = 0;
double steer = 0;

int ledstate = 0;

// Pan and tilt servos zero values and +/- angular range, in degrees:
#define PANZERO 90
#define PANRANGE 60
#define TILTZERO 70
#define TILTRANGE 40

// ###################################################################################################
// Simple PD servo controller
class ServoPD
{
public:
  ServoPD(long Kp, long Kd, long zero, long range, long scalebits = 8) :
  itsKp(Kp), itsKd(Kd), itsPos(zero << scalebits),
  itsPrevTarget(zero << scalebits), itsZero(zero << scalebits),
  itsRange(range << scalebits), itsScaleBits(scalebits)
  { }

  void attach(int pin, int pos)
  {
    itsServo.attach(pin);
    itsPos = (pos << itsScaleBits); 
    itsServo.write(pos);
  }
  
  long get() const
  {
    return (itsPos >> itsScaleBits);
  }
  
  void update(long targetpos)
  {
    targetpos <<= itsScaleBits;
    long diff = itsKp * targetpos + itsKd * (targetpos - itsPrevTarget);
    itsPos += (diff >> 16);
    itsPos = constrain(itsPos, itsZero - itsRange, itsZero + itsRange);
    itsServo.write(itsPos >> itsScaleBits);
    itsPrevTarget = targetpos;
  }

  void reset(long targetpos)
  {
    targetpos <<= itsScaleBits;
    itsPos = constrain(itsPos, itsZero - itsRange, itsZero + itsRange);
    itsPrevTarget = targetpos;
  }
  
  long rawget()
  {
    return itsPos >> itsScaleBits;
  }

  void rawset(long rawval)
  {
    itsPos = rawval << itsScaleBits;
    itsServo.write(rawval);
  }
  
  private:
  Servo itsServo;
  long itsKp, itsKd, itsPos, itsPrevTarget, itsZero, itsRange, itsScaleBits;
};

// Create one servo PD controler for camera pan and another for camera tilt:
ServoPD panservo(400, 200, PANZERO, PANRANGE);
ServoPD tiltservo(300, 100, TILTZERO, TILTRANGE);

// ###################################################################################################
void setup()
{
  SERIAL.setTimeout(50);
  SERIAL.begin(115200);
  SERIAL.setTimeout(50);
  pinMode(LPWMPIN, OUTPUT);
  pinMode(LIN1PIN, OUTPUT);
  pinMode(LIN2PIN, OUTPUT);
  pinMode(RPWMPIN, OUTPUT);
  pinMode(RIN1PIN, OUTPUT);
  pinMode(RIN2PIN, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(PANPIN, OUTPUT);
  pinMode(TILTPIN, OUTPUT);
  motor(0, 0);

  panservo.attach(PANPIN, PANZERO);
  tiltservo.attach(TILTPIN, TILTZERO);

  win = TARGW; wset = TARGW; wout = 0;
  wpid.SetMode(AUTOMATIC);
  wpid.SetSampleTime(10);
  wpid.SetOutputLimits(-120, 120);
  
  ain = 0; aset = 0; aout = 0;
  apid.SetMode(AUTOMATIC);
  apid.SetSampleTime(10);
  apid.SetOutputLimits(-50, 50);
}

// ###################################################################################################
void loop()
{
  byte len = SERIAL.readBytesUntil('\n', instr, INLEN);
  instr[len] = 0;

  char * tok = strtok(instr, " \r\n");
  int state = 0; int id, targx, targy, targw, targh;

  while (tok)
  {
    // State machine:
    // 0: start parsing
    // 1: N2 command, parse id
    // 2: N2 command, parse targx
    // 3: N2 command, parse targy
    // 4: N2 command, parse targw
    // 5: N2 command, parse targh
    // 6: N2 command complete
    // 1000: unknown command
    switch (state)
    {
      case 0: if (strcmp(tok, "N2") == 0) state = 1; else state = 1000; break;
      case 1: id = atoi(&tok[1]); state = 2; break; // ignore prefix
      case 2: targx = atoi(tok); state = 3; break;
      case 3: targy = atoi(tok); state = 4; break;
      case 4: targw = atoi(tok); state = 5; break;
      case 5: targh = atoi(tok); state = 6; break;
      default: break; // Skip any additional tokens
    }
    tok = strtok(0, " \r\n");
  }

  // If a complete new N2 command was received, act:
  if (state == 6)
  {
    // Actuate the pan/tilt head to track the target:
    panservo.update(-targx);
    tiltservo.update(targy);

    // Move the car forward/backward to track the derired target width:
    win = targw; wpid.Compute(); speed = wout;
    
    // Steer the car to zero out the camera's pan angle:
    ain = panservo.get() - PANZERO; apid.Compute(); steer = aout * (1.0 + fabs(speed) * 0.05);
  }
  else
  {
    // Slow down if we lost track:
    speed *= 0.95;
    steer *= 0.8;
  }
  
  // Actuate the motors:
  motor(speed + steer, speed - steer);

  // Blink the LED on every loop:
  digitalWrite(LEDPIN, ledstate);
  ledstate = !ledstate;
}

// ###################################################################################################
// Actuate the motors: Values in [-100 .. 100]; positive = forward, negative = backward, 0 = brake
void motor(long left, long right)
{
  if (left > 100) left = 100; else if (left < -100) left = -100;
  if (right > 100) right = 100; else if (right < -100) right = -100;
  
  long motleft = (left * leftgain) >> 8;
  long motright = (right * rightgain) >> 8;
  
  if (motleft > 0) {
      digitalWrite(LIN1PIN, LOW);
      digitalWrite(LIN2PIN, HIGH);
      analogWrite(LPWMPIN, motleft);
  } else if (motleft < 0) {
      digitalWrite(LIN1PIN, HIGH);
      digitalWrite(LIN2PIN, LOW);
      analogWrite(LPWMPIN, -motleft);
  } else {
      digitalWrite(LIN1PIN, LOW);
      digitalWrite(LIN2PIN, LOW);
  }
  
  if (motright > 0) {
      digitalWrite(RIN1PIN, LOW);
      digitalWrite(RIN2PIN, HIGH);
      analogWrite(RPWMPIN, motright);
  } else if (motright < 0) {
      digitalWrite(RIN1PIN, HIGH);
      digitalWrite(RIN2PIN, LOW);
      analogWrite(RPWMPIN, -motright);
  } else {
      digitalWrite(RIN1PIN, LOW);
      digitalWrite(RIN2PIN, LOW);
  }
}

