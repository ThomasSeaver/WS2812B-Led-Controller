#include <Arduino.h>
#include <FastLED.h>
#include <stdio.h>

// Define your led number and which pin you utilize to transmit data down the strip
#define NUM_LEDS 150
#define DATA_PIN 4

// Setup for serial handling
char serialData[64];
int serialIndex = 0;

// Setup for accessing LEDS and auto change features
CRGB leds[NUM_LEDS];
long previousMillis = 0;  
unsigned long interval = 500;
int colors[100][3];
int colorLast = -1;
int cf = 0;
int cl = NUM_LEDS;
int ci = 1;
int colorCur = 0;

// Setup for blending 
bool blending = false;
double blendPercent = 0.00;

// Setup for patterns
bool patterns = false;
int patternOffset = 0;

// Holding values for LED sets
int mRet = 0;
int rRet = 0;
int gRet = 0;
int bRet = 0;
int fRet = 0;
int lRet = NUM_LEDS;
int iRet = 1;

void(* resetFunc) (void) = 0; //declare reset function @ address 0 for programmatic reset

// Set and update our LEDs
void updateLeds(int r, int g, int b, int f, int l, int i) {
  // Output led info to serial for debug
  char inf[32];
  sprintf(inf, "     R: %3d G: %3d B: %3d F: %3d L: %3d I: %3d", r, g, b, f, l, i);
  Serial.println(inf);

  // Affect requested leds, and set the rest to no color value
  for (int led = 0; led < NUM_LEDS; led++) {
    if (led >= f && led < l && (led - f) % i == 0) {
      leds[led] = CRGB(r, g, b);
    } else {
      leds[led] = CRGB(0, 0, 0);
    }
  }
  FastLED.show();
}

// Set some of our LEDs
void setSomeLeds(int r, int g, int b, int f, int l, int i) {
  // Affect requested leds, and set the rest to no color value
  for (int led = f; led < l; led += i) {
    leds[led] = CRGB(r, g, b);
  }
}

void handleLedConfig(int m, int r, int g, int b, int f, int l, int i) {
  if (m==0) {
    Serial.println("Resetting Values");
    memset(colors, 1, sizeof(colors));
    colorLast = -1;
  } else if (m==1) {
    updateLeds(r,g,b,f,l,i);
    Serial.println("Setting LEDS");
  } else if (m==2) {
    colorLast++;
    colors[colorLast][0] = r;
    colors[colorLast][1] = g;
    colors[colorLast][2] = b;
  } else if (m==3 || m==4) {
    cf = f;
    cl = l;
    ci = i;
  } else if (m==5) {
    colors[0][0] = 255;
    colors[1][1] = 255;
    colors[2][2] = 255;
    colorLast = 2;
  } else if (m==9) {
    Serial.println("Resetting Arduino");
    resetFunc();
  }
  blending = (m == 3 && colorLast > 0);
  patterns = (m == 4 && colorLast > 0);
}

// Take in input string, and extract out various parameters 
int paramExtract(String data, int numlen, int valIndex, int lowBound, int upBound) {
  int retVal = 0;

  // Some numbers are multiple characters long, so go until we've exhausted length
  for (int n = 0; n < numlen; n++) {
    // Prevent ourselves from going past string's termination, shortcircuit if necessary
    if (valIndex < data.length() - (2 + n)) {
      // check that our potential character is a digit
      char potential = data.charAt(valIndex + (2 + n));
      if (isDigit(potential)) {
        // If it is a digit, convert to int, then add to our stored value
        // Since we're checking left to right, we left shift our previous values by an order of magnitude
        int newNum = potential - '0';
        retVal = retVal * 10;
        retVal += newNum;
      }
    } else {
      n = numlen;
    }
  }
  
  // Check that our potential input is within the bounds as specified by the parameters, otherwise return 0
  if (retVal >= lowBound && retVal <= upBound) {
    return retVal;
  } else {
    return 0;
  }
}

// Handle our serial data as it comes in
void handleSerial(char serialData[64], int serialIndex) {
  // Convert to a string
  String data = serialData;
  // Check string for each possible parameter, and if it exists, run it through our extraction function, otherwise keep pre-existing config
  mRet = (data.indexOf('m') >= 0) ? paramExtract(data, 1, data.indexOf('m'), 0, 9) : mRet;
  rRet = (data.indexOf('r') >= 0) ? paramExtract(data, 3, data.indexOf('r'), 0, 255) : rRet;
  gRet = (data.indexOf('g') >= 0) ? paramExtract(data, 3, data.indexOf('g'), 0, 255) : gRet;
  bRet = (data.indexOf('b') >= 0) ? paramExtract(data, 3, data.indexOf('b'), 0, 255) : bRet;
  fRet = (data.indexOf('f') >= 0) ? paramExtract(data, 3, data.indexOf('f'), 0, NUM_LEDS) : fRet;
  lRet = (data.indexOf('l') >= 0) ? paramExtract(data, 3, data.indexOf('l'), 0, NUM_LEDS) : lRet;
  iRet = (data.indexOf('i') >= 0) ? paramExtract(data, 3, data.indexOf('i'), 0, NUM_LEDS) : iRet;

  // Output updated config info
  char inf[32];
  sprintf(inf, "M: %d R: %3d G: %3d B: %3d F: %3d L: %3d I: %3d", mRet, rRet, gRet, bRet, fRet, lRet, iRet);
  Serial.println(inf);

  // If z exists in our input, we know this means to set leds 
  int updateLeds = data.indexOf('z');
  if (updateLeds >= 0) {
    handleLedConfig(mRet, rRet, gRet, bRet, fRet, lRet, iRet);
  }
}

// Handler function for automatically generating rgb blends between different rgb values
void blend() {
  int blendNext = (colorCur + 1) % (colorLast + 1);
  // Generate our mid-state colors between blend states
  int r = colors[colorCur][0] + ((colors[blendNext][0] - colors[colorCur][0]) * blendPercent);
  int g = colors[colorCur][1] + ((colors[blendNext][1] - colors[colorCur][1]) * blendPercent);
  int b = colors[colorCur][2] + ((colors[blendNext][2] - colors[colorCur][2]) * blendPercent);

  // Update our blend percentage, reset it if we are into the next state and also move towards next state
  // Increments by 0.004 because this is approximately 1/255, so colors will be changing by at least 1 every interval
  blendPercent += 0.004;
  if (blendPercent >= 1) {
    blendPercent = 0.00;
    colorCur = blendNext;
  }

  // Update our leds with mid-state colors
  updateLeds(r,g,b,cf,cl,ci);
}

// Handler function for automatically generating Led patterns
void pattern() {
  int pi = ci + colorLast;
  setSomeLeds(0,0,0,0,NUM_LEDS,1);
  // Build pattern
  for (int color = 0; color <= colorLast; color++) {
    int cind = (color + patternOffset) % (colorLast + 1);
    setSomeLeds(colors[cind][0],colors[cind][1],colors[cind][2],cf+color,cl,pi);
  }
  // Update leds, move offset along
  FastLED.show();
  patternOffset++;
}

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  // Visible test that strip is working
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Red;
  leds[2] = CRGB::Red;
  FastLED.show();
  // Initialize LEDS, Baud Rate, and Wifi configs
  Serial.begin(115200);
  Serial.flush();
}

void loop() {
  // If serial information available to collect, read/store it 
  while (Serial.available() > 0 && serialIndex != 64) {
    serialData[serialIndex] = Serial.read(); // Read incoming byte
    serialIndex++; // Prepare for next byte
  }    
  // if our buffer is full or new line is the latest val in the buffer
  if (serialIndex == 64 || (serialIndex != 0 && serialData[serialIndex - 1] == '\n')){
    serialData[serialIndex - 1] = '\0'; // Terminate properly
    // Pass our buffer to our serial data handling function
    handleSerial(serialData, serialIndex);
    // Reset our buffer/index
    serialIndex = 0;
    memset(serialData, 0, sizeof serialData);
  }

  // On specific interval, update LEDs if blend/pattern mode is on
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;

    if (blending) {
      blend();
    }
    if (patterns) {
      pattern();
    }
  }
}