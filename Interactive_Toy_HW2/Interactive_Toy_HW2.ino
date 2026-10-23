const int RledPin = 9;   // the PWM pin the Red LED is attached to
const int GledPin = 10;  // the PWM pin the Green LED is attached to
const int BledPin = 11;  // the PWM pin the Blue LED is attached to

const int buttonPin = 2;         // the pin that the pushbutton is attached to
int buttonState = 0;             // Button Current State
bool ButtonPressed = false;      // Has the button been pressed or not

int mood = 10;                   // the toy's mood state (starts at 10 / neutral)
const int neutralMood = 10;      // the neutral mood state

unsigned long previousMillis = 0;      // stores last time LED was updated
unsigned long touchedTimer = 0;        // the timer used to record the last touch time
unsigned long reducedTimer = 0;        // the timer used to record the last reduced point time
const long unTouchInterval = 5000;     // the interval before the toy starts getting sad (5 seconds)
const long reducedInterval = 1000;     // interval to decrease mood by 1 every second

// Independent variables for breathing effects
unsigned long sadBreathingPrevMillis = 0;
int sadBreathingBrightness = 255;
int sadBreathingDirection = -15;       // Fast step size for an urgent warning pulse

unsigned long happyBreathingPrevMillis = 0;
int happyBreathingBrightness = 255;
int happyBreathingDirection = -5;      // Slow, gentle step size for a calm vibe

// State transition tracking flags to prevent freezing on repeated clicks
bool wasAtZero = false;
bool wasAtTwenty = false;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);

  // Initialize timers
  touchedTimer = millis();
}

void loop() {
  showLEDState(mood);

  // Check if the button is pressed or not
  buttonState = digitalRead(buttonPin);

  // If the button is clicked (petting the toy)
  if (buttonState == HIGH && !ButtonPressed) {
    mood = mood + 1;
    if (mood > 20) mood = 20;  // Cap maximum mood at 20 (Happy)

    // Reset the untouched timer whenever the user pets the toy
    touchedTimer = millis();
    ButtonPressed = true;
  }

  if (buttonState == LOW && ButtonPressed) {
    ButtonPressed = false;
  }

  // Check if the user hasn't touched the toy for longer than 5 seconds
  unsigned long currentTimer = millis();
  if (currentTimer - touchedTimer > unTouchInterval) {
    // Decrease mood by 1 point every second
    if (currentTimer - reducedTimer > reducedInterval) {
      mood = mood - 1;
      if (mood < 0) mood = 0;  // Floor minimum mood at 0 (Sad)
      reducedTimer = currentTimer;
    }
  }
}

// Function configured for Common Anode RGB LEDs with fixed breathing reset logic
void showLEDState(int state) {
  float brightnessInterval = 255 / 10.0;
  int redVal = 255;
  int greenVal = 255;
  int blueVal = 255;

  // Continuous color spectrum calculation (Common Anode Inversion)
  if (state >= neutralMood) {
    // Neutral (10) to Happy (20): Transition from Green to Blue
    redVal = 255;
    greenVal = brightnessInterval * (state - neutralMood);
    blueVal = 255 - (brightnessInterval * (state - neutralMood));
  }
  else {
    // Sad (0) to Neutral (10): Transition from Red to Green
    redVal = 255 - (brightnessInterval * (neutralMood - state));
    greenVal = brightnessInterval * (neutralMood - state);
    blueVal = 255;
  }

  // SAD STATE (0 points): Fast warning pulse
  if (state == 0) {
    if (!wasAtZero) {
      sadBreathingBrightness = 255;
      sadBreathingDirection = -15;          // reset direction on entry
      sadBreathingPrevMillis = millis();
      wasAtZero = true;
    }

    unsigned long currentMillis = millis();
    if (currentMillis - sadBreathingPrevMillis > 12) {
      sadBreathingBrightness += sadBreathingDirection;
      if (sadBreathingBrightness <= 20) {
        sadBreathingBrightness = 20;
        sadBreathingDirection = abs(sadBreathingDirection);    // go up
      } else if (sadBreathingBrightness >= 255) {
        sadBreathingBrightness = 255;
        sadBreathingDirection = -abs(sadBreathingDirection);   // go down
      }
      sadBreathingPrevMillis = currentMillis;
    }
    int intensityOffset = 255 - sadBreathingBrightness;
    redVal = constrain(redVal + intensityOffset, 0, 255);
  }
  else {
    wasAtZero = false;
  }

  // HAPPY STATE (20 points): Calm breathing pulse
  if (state == 20) {
    if (!wasAtTwenty) {
      happyBreathingBrightness = 255;
      happyBreathingDirection = -5;         // reset direction on entry
      happyBreathingPrevMillis = millis();
      wasAtTwenty = true;
    }

    unsigned long currentMillis = millis();
    if (currentMillis - happyBreathingPrevMillis > 30) {
      happyBreathingBrightness += happyBreathingDirection;
      if (happyBreathingBrightness <= 60) {
        happyBreathingBrightness = 60;
        happyBreathingDirection = abs(happyBreathingDirection);
      } else if (happyBreathingBrightness >= 255) {
        happyBreathingBrightness = 255;
        happyBreathingDirection = -abs(happyBreathingDirection);
      }
      happyBreathingPrevMillis = currentMillis;
    }
    int intensityOffset = 255 - happyBreathingBrightness;
    blueVal = constrain(blueVal + intensityOffset, 0, 255);
  }
  else {
    wasAtTwenty = false;
  }

  // Output PWM signals to the RGB LED
  analogWrite(RledPin, redVal);
  analogWrite(GledPin, greenVal);
  analogWrite(BledPin, blueVal);
}