
const int rLEDPin = 4;
const int gLEDPin = 5;
const int bLEDPin = 6; 


const int buttonPin = 2;


float currentR = 255;
float currentG = 255;
float currentB = 255;


int colorMode = 1; 
int lastButtonState = HIGH;
bool modeChanged = false; 

void setup() {
  pinMode(rLEDPin, OUTPUT);
  pinMode(gLEDPin, OUTPUT);
  pinMode(bLEDPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 


  setColor(255, 255, 255);
}

void loop() {

  modeChanged = false;

  if (colorMode == 1) {

    fadeToColor(0, 255, 255, 1000);   
    if (modeChanged) return;          
    delayWithButtonCheck(500);
    if (modeChanged) return;

    fadeToColor(0, 128, 255, 1000);  
    if (modeChanged) return;
    delayWithButtonCheck(500);
    if (modeChanged) return;

    fadeToColor(0, 0, 255, 1000);     
    if (modeChanged) return;
    delayWithButtonCheck(500);
  } 
  else {
 
    fadeToColor(255, 0, 255, 1000);  
    if (modeChanged) return;
    delayWithButtonCheck(500);
    if (modeChanged) return;

    fadeToColor(255, 255, 0, 1000);  
    if (modeChanged) return;
    delayWithButtonCheck(500);
    if (modeChanged) return;

    fadeToColor(0, 255, 0, 1000);     
    if (modeChanged) return;
    delayWithButtonCheck(500);
  }
}


void checkButton() {
  int buttonState = digitalRead(buttonPin);
  
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(50); 
    if (digitalRead(buttonPin) == LOW) {
      colorMode = (colorMode == 1) ? 2 : 1; 
      modeChanged = true;                   
    }
  }
  lastButtonState = buttonState;
}


void delayWithButtonCheck(int durationMs) {
  int steps = durationMs / 10;
  for (int i = 0; i < steps; i++) {
    checkButton();
    if (modeChanged) return;
    delay(10);
  }
}


void setColor(int r, int g, int b) {
  analogWrite(rLEDPin, r);
  analogWrite(gLEDPin, g);
  analogWrite(bLEDPin, b);
  currentR = r;
  currentG = g;
  currentB = b;
}


void fadeToColor(int targetR, int targetG, int targetB, int durationMs) {
  int steps = 50;
  float delayPerStep = (float)durationMs / steps;

  float stepR = (targetR - currentR) / steps;
  float stepG = (targetG - currentG) / steps;
  float stepB = (targetB - currentB) / steps;

  for (int i = 0; i < steps; i++) {
    checkButton();
    if (modeChanged) return; 

    currentR += stepR;
    currentG += stepG;
    currentB += stepB;

    analogWrite(rLEDPin, (int)currentR);
    analogWrite(gLEDPin, (int)currentG);
    analogWrite(bLEDPin, (int)currentB);

    delay((int)delayPerStep);
  }

  setColor(targetR, targetG, targetB);
}