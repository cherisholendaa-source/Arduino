const int buttonPin = 2; // Button connected to digital pin 2
const int ledPin = 3;    // LED connected to digital pin 3

int buttonState = 0;
bool buttonPressed = false;

// Mode State:
// 0: Steady lighting (衡亮) -> LOW = ON
// 1: Slow blink (慢閃爍 - 1000ms)
// 2: Medium blink (不快不慢的閃爍 - 500ms)
// 3: Fast blink (快閃爍 - 200ms)
int currentMode = 0;

unsigned long blinkTimer = 0;
bool blinkOn = true;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  checkButton();
  updateLEDState();
}

void checkButton() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && !buttonPressed) {
    buttonPressed = true;
  }

  if (buttonState == LOW && buttonPressed) {
    currentMode = (currentMode + 1) % 4; 
    
    Serial.print("Button Clicked! Current Mode: ");
    Serial.println(currentMode);
    
    buttonPressed = false;
  }
}

void updateLEDState() {
  unsigned long currentTime = millis();

  if (currentMode == 0) {
    // 1. Initial State: Steady lighting (LOW turns it ON)
    digitalWrite(ledPin, LOW);
  } 
  else if (currentMode == 1) {
    // 2. Slow blink (1000ms interval)
    if (currentTime - blinkTimer >= 1000) {
      blinkTimer = currentTime;
      blinkOn = !blinkOn;
    }
    digitalWrite(ledPin, blinkOn ? LOW : HIGH);
  } 
  else if (currentMode == 2) {
    // 3. Medium blink (500ms interval)
    if (currentTime - blinkTimer >= 500) {
      blinkTimer = currentTime;
      blinkOn = !blinkOn;
    }
    digitalWrite(ledPin, blinkOn ? LOW : HIGH);
  } 
  else if (currentMode == 3) {
    // 4. Fast blink (200ms interval)
    if (currentTime - blinkTimer >= 200) {
      blinkTimer = currentTime;
      blinkOn = !blinkOn;
    }
    digitalWrite(ledPin, blinkOn ? LOW : HIGH);
  }
}