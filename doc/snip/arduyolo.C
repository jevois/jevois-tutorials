// JeVois + Arduino blink for X from YOLO

// Pin for LED, will turn on as we detect the desired object:
#define LEDPIN 17

// Serial port to use: on chips with USB (e.g., 32u4), that usually is Serial1.
// On chips without USB, use Serial:
#define SERIAL Serial1

// Buffer for received serial port bytes:
#define INLEN 256
char instr[INLEN + 1];

// Our desired object: should be one of the 1000 ImageNet category names
#define CATEGORY "dog"

// Our desired minimum object width in standardized coordinates:
#define MIN_WIDTH 200.0F

void setup()
{
  SERIAL.begin(115200);
  SERIAL.setTimeout(500);
  
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
}

void loop()
{
  byte len = SERIAL.readBytesUntil('\n', instr, INLEN);
  instr[len] = 0;

  char * tok = strtok(instr, " \r\n");
  int state = 0, i; float score, left, top, width, height;
  
  while (tok)
  {
    // State machine:
    // 0: start parsing; if we get N2, move to state 1, otherwise state 1000
    // 1: decode category name; if it is the one we want, move to state 2, otherwise state 1000
    // 2: decode left and move to state 3
    // 3: decode top and move to state 4
    // 4: decode width and move to state 5
    // 5: decode height and move to state 6
    // 6: we got a full message, stay in this state until we run out of tokens
    // 1000: we stay in this state until we run out of tokens
    switch (state)
    {
      // First token should be: N2
    case 0:
      if (strcmp(tok, "N2") == 0) state = 1; else state = 1000;
      // We are done with this token. Break from the switch() statement
      break;
      
      // Second token should be: category:score
    case 1:
      // Find the ':' between category and score:
      i = strlen(tok) - 1;
      while (i >= 0 && tok[i] != ':') --i;
      
      // If i is >= 0, we found a ':'; terminate the tok string at that ':':
      if (i >= 0)
      {
        tok[i] = '\0';
        score = atof(&tok[i+1]);
      }
      
      // Is the category name what we want?
      if (strcmp(tok, CATEGORY) == 0) state = 2; else state = 1000;
      
      // We are done with this token. Break from the switch() statement
      break;
      
      // Third token: left
    case 2:
      left = atof(tok);
      state = 3;
      break;
      
      // Fourth token: top
    case 3:
      top = atof(tok);
      state = 4;
      break;
      
      // Fifth token: width
    case 4:
      width = atof(tok);
      state = 5;
      break;
      
      // Sixth token: height
    case 5:
      height = atof(tok);
      state = 6;
      // We got a whole message!
      break;
      
      // In any other state: do nothing
    default:
      break;
    }
    
    // Move to the next token:
    tok = strtok(0, " \r\n");
  }
  
  // If we are in state 6, we successfully parsed a whole message and the category is a match.
  // We just need to test the width and activate the LED accordingly:
  if (state == 6 && width >= MIN_WIDTH)
    digitalWrite(LEDPIN, LOW); // turn LED on (it has inverted logic)
  else
    digitalWrite(LEDPIN, HIGH); // turn LED off
}
