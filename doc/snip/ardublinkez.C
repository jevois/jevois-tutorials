// JeVois + Arduino blink for X - easy version

// Pin for LED, blinks as we receive serial commands:
#define LEDPIN 17

// Serial port to use: on chips with USB (e.g., 32u4), that usually is Serial1.
// On chips without USB, use Serial:
#define SERIAL Serial1

// Buffer for received serial port bytes:
#define INLEN 256
char instr[INLEN + 1];

// Our desired object: should be one of the 1000 ImageNet category names
#define CATEGORY "computer_keyboard"

void setup()
{
  SERIAL.begin(115200);
  SERIAL.setTimeout(500);
  
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
}

void loop()
{
  // Read a line of data from JeVois:
  byte len = SERIAL.readBytesUntil('\n', instr, INLEN);

  // Make sure the string is null-terminated:
  instr[len--] = '\0';
  
  // Cleanup any trailing whitespace:
  while (len >= 0 && instr[len] == ' ' || instr[len] == '\r' || instr[len] == '\n') instr[len--] = '\0';
  
  // If empty (including timeout), stop here:
  if (len < 0)
  {
    digitalWrite(LEDPIN, HIGH); // turn LED off (it has inverted logic)
    return;
  }
  
  // If the message is a match for our desired category, turn led on, otherwise off:
  if (strcmp(instr, "TO " CATEGORY) == 0)
    digitalWrite(LEDPIN, LOW); // turn LED on (it has inverted logic)
  else
    digitalWrite(LEDPIN, HIGH); // turn LED off
}
