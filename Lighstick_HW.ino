// ================= Light Stick =================
// 1 click        : change color
// 2 clicks       : Rainbow mode (smooth color cycle)
// 3 clicks       : Rainbow Blink mode (secret mode)
// Hold 2s+, release : next mode  (LED blinks fast once 2s is reached)
// Keep holding to 5s : power ON/OFF (happens while still holding)

// ---------------- Pins ----------------
const int buttonPin = 2;
const int RLedPin = 9;
const int GLedPin = 10;
const int BLedPin = 11;

// Common pin on 3.3V -> common anode -> 0 = ON.
// If colors look inverted, or "off" looks like white, flip this to false.
const bool COMMON_ANODE = true;

// ---------------- Timing ----------------
const unsigned long debounceDelay  = 30;
const unsigned long modePressTime  = 2000;  // hold + release -> mode switch
const unsigned long powerPressTime = 5000;  // keep holding   -> power toggle
const unsigned long clickGap       = 500;   // max gap between clicks in a multi-click

// ---------------- Button state ----------------
int pressedLevel = HIGH;       // auto-detected in setup()
int lastReading = LOW;
unsigned long lastChange = 0;
bool btnDown = false;
bool powerHandled = false;     // power already toggled during this press
unsigned long pressStart = 0;

int clickCount = 0;            // clicks collected so far
unsigned long lastClickTime = 0;

// ---------------- Power ----------------
bool isPowerOn = true;

// ---------------- Modes ----------------
// 0 Constant, 1 Blink, 2 Breathing, 3 Rainbow, 4 Rainbow Blink (secret)
int currentMode = 0;

unsigned long blinkTimer = 0;
const unsigned long blinkInterval = 500;
bool blinkOn = true;

int fadeValue = 0;
int fadeDirection = 5;
unsigned long fadeTimer = 0;
const unsigned long fadeInterval = 30;

// Rainbow: hue wheel 0..1535 (6 segments of 256)
int rainbowHue = 0;
unsigned long rainbowTimer = 0;
const unsigned long rainbowInterval = 20;
const unsigned long rainbowBlinkInterval = 200;

// ---------------- Colors ----------------
int lightNum = 0;
int RLightColor = 255, GLightColor = 255, BLightColor = 255;
int RCurrentColor = 0, GCurrentColor = 0, BCurrentColor = 0;

void setup()
{
    pinMode(buttonPin, INPUT);
    pinMode(RLedPin, OUTPUT);
    pinMode(GLedPin, OUTPUT);
    pinMode(BLedPin, OUTPUT);
    Serial.begin(9600);

    // Detect which level means "pressed": whatever the idle level is,
    // the opposite is a press. Don't touch the button during startup.
    delay(200);
    int idleLevel = digitalRead(buttonPin);
    pressedLevel = (idleLevel == HIGH) ? LOW : HIGH;
    lastReading = idleLevel;

    Serial.print("Idle level: ");
    Serial.println(idleLevel == HIGH ? "HIGH (button is active-LOW)" : "LOW (button is active-HIGH)");
}

void loop()
{
    checkButton();
    handlePendingClick();
    updateLEDColor();
    applyHoldFeedback();
    setRGBLEDColor(RCurrentColor, GCurrentColor, BCurrentColor);
}

// ---------------- Button ----------------
void checkButton()
{
    int reading = digitalRead(buttonPin);

    if (reading != lastReading) {
        lastChange = millis();
        lastReading = reading;
    }

    if ((millis() - lastChange) > debounceDelay) {
        if (reading == pressedLevel && !btnDown) {
            btnDown = true;
            powerHandled = false;
            pressStart = millis();
            Serial.println("Button down");
        }
        else if (reading != pressedLevel && btnDown) {
            btnDown = false;
            onRelease(millis() - pressStart);
        }
    }

    // Still holding: toggle power the moment 5 s is reached
    if (btnDown && !powerHandled && (millis() - pressStart) >= powerPressTime) {
        powerHandled = true;
        clickCount = 0;
        isPowerOn = !isPowerOn;
        Serial.println(isPowerOn ? "-> POWER ON" : "-> POWER OFF");
    }
}

void onRelease(unsigned long duration)
{
    Serial.print("Released after ");
    Serial.print(duration);
    Serial.println(" ms");

    if (powerHandled) return;     // this press was the power toggle
    if (!isPowerOn) return;       // everything else is ignored while off

    if (duration >= modePressTime) {   // long press -> next mode
        clickCount = 0;
        changeMode();
        return;
    }

    // Short click: count it
    clickCount++;
    lastClickTime = millis();
    if (clickCount >= 3) runClicks();  // triple click fires immediately
}

// Fires after the click window closes (1 or 2 clicks)
void handlePendingClick()
{
    if (clickCount > 0 && (millis() - lastClickTime) >= clickGap) {
        runClicks();
    }
}

void runClicks()
{
    if (clickCount == 1) {
        Serial.println("-> 1 click: change color");
        changeLEDColor();
    } else if (clickCount == 2) {
        Serial.println("-> 2 clicks: Rainbow");
        setMode(3);
    } else {
        Serial.println("-> 3 clicks: Rainbow Blink");
        setMode(4);
    }
    clickCount = 0;
}

// While holding past 2 s, blink fast so you can see the hold registered.
// Release now = next mode. Keep holding = power toggle at 5 s.
void applyHoldFeedback()
{
    if (btnDown && !powerHandled && (millis() - pressStart) >= modePressTime) {
        if (((millis() / 100) % 2) == 0) {
            RCurrentColor = RLightColor;
            GCurrentColor = GLightColor;
            BCurrentColor = BLightColor;
        } else {
            RCurrentColor = GCurrentColor = BCurrentColor = 0;
        }
    }
}

// ---------------- Modes ----------------
void changeMode()
{
    // 0 -> 1 -> 2 -> 3 -> 0 ; secret mode 4 goes back to 0
    setMode(currentMode >= 3 ? 0 : currentMode + 1);
}

void setMode(int mode)
{
    currentMode = mode;
    if (currentMode == 1) { blinkTimer = millis(); blinkOn = true; }
    if (currentMode == 2) { fadeValue = 0; fadeDirection = 5; fadeTimer = millis(); }
    if (currentMode == 3 || currentMode == 4) {
        rainbowHue = 0;
        rainbowTimer = millis();
        blinkTimer = millis();
        blinkOn = true;
    }
    Serial.print("Mode: ");
    Serial.println(currentMode);
}

// Full-brightness hue wheel: one channel is always 255, so no dark spots
void hueToRGB(int h, int &r, int &g, int &b)
{
    int seg = h / 256;
    int x = h % 256;
    switch (seg) {
        case 0:  r = 255;     g = x;       b = 0;       break;
        case 1:  r = 255 - x; g = 255;     b = 0;       break;
        case 2:  r = 0;       g = 255;     b = x;       break;
        case 3:  r = 0;       g = 255 - x; b = 255;     break;
        case 4:  r = x;       g = 0;       b = 255;     break;
        default: r = 255;     g = 0;       b = 255 - x; break;
    }
}

void updateLEDColor()
{
    if (!isPowerOn) {
        RCurrentColor = GCurrentColor = BCurrentColor = 0;
        return;
    }

    unsigned long now = millis();

    if (currentMode == 0) {
        RCurrentColor = RLightColor;
        GCurrentColor = GLightColor;
        BCurrentColor = BLightColor;
    }
    else if (currentMode == 1) {
        if (now - blinkTimer >= blinkInterval) {
            blinkOn = !blinkOn;
            blinkTimer = now;
        }
        RCurrentColor = blinkOn ? RLightColor : 0;
        GCurrentColor = blinkOn ? GLightColor : 0;
        BCurrentColor = blinkOn ? BLightColor : 0;
    }
    else if (currentMode == 2) {
        if (now - fadeTimer >= fadeInterval) {
            fadeTimer = now;
            fadeValue += fadeDirection;
            if (fadeValue >= 255) { fadeValue = 255; fadeDirection = -fadeDirection; }
            if (fadeValue <= 0)   { fadeValue = 0;   fadeDirection = -fadeDirection; }
        }
        RCurrentColor = (RLightColor * fadeValue) / 255;
        GCurrentColor = (GLightColor * fadeValue) / 255;
        BCurrentColor = (BLightColor * fadeValue) / 255;
    }
    else if (currentMode == 3) {
        // Smooth rainbow, never dark
        if (now - rainbowTimer >= rainbowInterval) {
            rainbowTimer = now;
            rainbowHue = (rainbowHue + 6) % 1536;
        }
        hueToRGB(rainbowHue, RCurrentColor, GCurrentColor, BCurrentColor);
    }
    else if (currentMode == 4) {
        // Rainbow Blink: hue cycles faster and the light blinks
        if (now - rainbowTimer >= rainbowInterval) {
            rainbowTimer = now;
            rainbowHue = (rainbowHue + 12) % 1536;
        }
        if (now - blinkTimer >= rainbowBlinkInterval) {
            blinkOn = !blinkOn;
            blinkTimer = now;
        }
        if (blinkOn) {
            hueToRGB(rainbowHue, RCurrentColor, GCurrentColor, BCurrentColor);
        } else {
            RCurrentColor = GCurrentColor = BCurrentColor = 0;
        }
    }
}

void changeLEDColor()
{
    lightNum = (lightNum + 1) % 5;
    switch (lightNum) {
        case 0: RLightColor = 255; GLightColor = 255; BLightColor = 255; break; // white
        case 1: RLightColor = 255; GLightColor = 0;   BLightColor = 0;   break; // red
        case 2: RLightColor = 0;   GLightColor = 255; BLightColor = 0;   break; // green
        case 3: RLightColor = 0;   GLightColor = 0;   BLightColor = 255; break; // blue
        case 4: RLightColor = 255; GLightColor = 255; BLightColor = 0;   break; // yellow
    }
}

// Handles common-anode inversion in one place
void setRGBLEDColor(int r, int g, int b)
{
    if (COMMON_ANODE) {
        r = 255 - r;
        g = 255 - g;
        b = 255 - b;
    }
    analogWrite(RLedPin, r);
    analogWrite(GLedPin, g);
    analogWrite(BLedPin, b);
}