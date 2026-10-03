// PWM pins (marked with ~ on Arduino)
const int rLEDPin = 3;
const int gLEDPin = 4;
const int bLEDPin = 5; 

// Current RGB brightness state (0 = ON, 255 = OFF for Common Anode)
float currentR = 255;
float currentG = 255;
float currentB = 255;

void setup() {
  pinMode(rLEDPin, OUTPUT);
  pinMode(gLEDPin, OUTPUT);
  pinMode(bLEDPin, OUTPUT);

  // Start with LED off
  setColor(255, 255, 255);
}

void loop() {
  // Red (R: ON, G: OFF, B: OFF)
  fadeToColor(0, 255, 255, 1500);
  delay(500);

  // White (R: ON, G: ON, B: ON)
  fadeToColor(0, 0, 0, 1500);
  delay(500);

  // Blue (R: OFF, G: OFF, B: ON)
  fadeToColor(255, 255, 0, 1500);
  delay(500);

  // Yellow (R: ON, G: ON, B: OFF)
  fadeToColor(0, 0, 255, 1500);
  delay(500);
}

// Function to directly set pin outputs
void setColor(int r, int g, int b) {
  analogWrite(rLEDPin, r);
  analogWrite(gLEDPin, g);
  analogWrite(bLEDPin, b);
  currentR = r;
  currentG = g;
  currentB = b;
}

// Function to smoothly transition from current color to target color
void fadeToColor(int targetR, int targetG, int targetB, int durationMs) {
  int steps = 100;
  float delayPerStep = (float)durationMs / steps;

  float stepR = (targetR - currentR) / steps;
  float stepG = (targetG - currentG) / steps;
  float stepB = (targetB - currentB) / steps;

  for (int i = 0; i < steps; i++) {
    currentR += stepR;
    currentG += stepG;
    currentB += stepB;

    analogWrite(rLEDPin, (int)currentR);
    analogWrite(gLEDPin, (int)currentG);
    analogWrite(bLEDPin, (int)currentB);

    delay((int)delayPerStep);
  }

  // Ensure target color is exact at the end
  setColor(targetR, targetG, targetB);
}