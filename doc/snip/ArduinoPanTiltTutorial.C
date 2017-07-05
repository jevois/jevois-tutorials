// JeVois control steering or a pan/tilt head from the output of JeVois modules
//
// We handle messages "T2 <targetx> <targety>", "T1 <targetx>", "PANGAIN <gain>", and "TILTGAIN <gain>".
// targetx and targety are assumed to be in the -1000 ... 1000 range as output by the JeVois Kalman filters.
// Here we only do simple PD control under the assumption that target coordinates have already been filtered upstream.
#include <Servo.h>
// Pin for LED, blinks as we receive serial commands:
#define LEDPIN 13
// Serial port to use: on chips with USB (e.g., 32u4), that usually is Serial1. On chips without USB, use Serial:
#define SERIAL Serial1
// Pins for up to two servos:
Servo panservo;
#define PANPIN 3
Servo tiltservo;
#define TILTPIN 5
// Initial servo values in degrees:
#define PANZERO 90
#define TILTZERO 90
// With updates typically coming in at 60Hz or up to 120Hz, we will often need to move by a fraction of a
// degree. Hence we keep track of the pan and tilt values multiplied by SCALE. For the gains, a gain of 100
// means we will update servo angle by the 0.1*(target value/SCALE) degrees on each update. Higher gains mean
// larger angular updates.
#define SCALE 100
long pangain = 100;
long tiltgain = 100;
long panval = PANZERO * SCALE;
long tiltval = TILTZERO * SCALE;
// Buffer for received serial port bytes:
#define INLEN 128
char instr[INLEN + 1];
void setup()
{
  SERIAL.begin(115200);
  SERIAL.setTimeout(1000000);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  
  panservo.attach(PANPIN);
  panservo.write(panval / SCALE);
  tiltservo.attach(TILTPIN);
  tiltservo.write(tiltval / SCALE);
  // We are ready to rock, disable logs and turn on serial outputs on JeVois platform:
  SERIAL.println("setpar serlog None");
  SERIAL.println("setpar serout Hard");
}
void loop()
{
  digitalWrite(LEDPIN, LOW);
  byte len = SERIAL.readBytesUntil('\n', instr, INLEN);
  instr[len] = 0;
  digitalWrite(LEDPIN, HIGH);
  char * tok = strtok(instr, " \r\n");
  int state = 0; int targx = 0, targy = 0;
  while (tok)
  {
    // State machine:
    // 0: start parsing
    // 1: T2 command, parse targx
    // 2: T2 command, parse targy
    // 3: T2 command complete
    // 4: T1 command, parse targx
    // 5: T1 command complete
    // 6: PANGAIN command, parse pangain
    // 7: PANGAIN command complete
    // 8: TILTGAIN command, parse tiltgain
    // 9: TILTGAIN command complete
    // 1000: unknown command
    switch (state)
    {
      case 0:
        if (strcmp(tok, "T2") == 0) state = 1;
        else if (strcmp(tok, "T1") == 0) state = 4;
        else if (strcmp(tok, "PANGAIN") == 0) state = 6;
        else if (strcmp(tok, "TILTGAIN") == 0) state = 8;
        else state = 1000;
        break;
        
      case 1: targx = atoi(tok); state = 2; break;
      case 2: targy = atoi(tok); state = 3; break;
      case 4: targx = atoi(tok); state = 5; break;
      case 6: pangain = atoi(tok); state = 7; break;
      case 8: tiltgain = atoi(tok); state = 9; break;
      default: break; // Skip any additional tokens
    }
    tok = strtok(0, " \r\n");
  }
  // Target coordinates are in range -1000 ... 1000. Servos want 0 ... 180.
  // We also need to negate as needed so that the servo turns to cancel any offset from center:
  if (state == 3 || state == 5)
  {
    panval -= (targx * pangain) / 1000;
    if (panval < 5 * SCALE) panval = 5 * SCALE; else if (panval > 175 * SCALE) panval = 175 * SCALE;
    panservo.write(panval / SCALE);
  }
  
  if (state == 3)
  {
    tiltval += (targy * tiltgain) / 1000;
    if (tiltval < 5 * SCALE) tiltval = 5 * SCALE; else if (tiltval > 175 * SCALE) tiltval = 175 * SCALE;
    tiltservo.write(tiltval / SCALE);
  }
}
