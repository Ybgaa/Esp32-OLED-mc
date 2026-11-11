
#include <BleKeyboard.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <string.h>

// =============== BLUETOOTH ===============
BleKeyboard bleKeyboard("Desk Buddy", "Joe Projects", 100);

// =============== OLED ===============
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =============== TOUCH SENSOR ===============
#define TOUCH_PIN 27
#define TOUCH_ACTIVE HIGH

const unsigned long DEBOUNCE_MS = 45;
const unsigned long DOUBLE_CLICK_MS = 350;
const unsigned long LONG_PRESS_MS = 700;
const unsigned long MESSAGE_MS = 1000;
const unsigned long FRAME_INTERVAL_MS = 50;

// =============== TOUCH STATE ===============
bool lastReading = LOW;
bool stableState = LOW;
bool touchPressed = false;
bool longPressHandled = false;

unsigned long lastChangeTime = 0;
unsigned long pressStartTime = 0;
unsigned long firstTapTime = 0;

bool waitingForSecondTap = false;

// =============== DISPLAY STATE ===============
bool showingMessage = false;
unsigned long messageStartTime = 0;

char currentMessage[24] = "";

// =============== CD ANIMATION ===============
unsigned long lastAnimationTime = 0;
int cdAngle = 0;

// ------------------------------------------------
// DRAW SPINNING CD
// ------------------------------------------------
void drawCD() {
  display.clearDisplay();

  const int cx = 64;
  const int cy = 32;
  const int radius = 25;

  // Outer edge of the CD
  display.drawCircle(cx, cy, radius, WHITE);
  display.drawCircle(cx, cy, radius - 2, WHITE);

  // Rotating reflective streaks
  for (int i = 0; i < 4; i++) {
    float angle = (cdAngle + i * 90) * 0.0174533f;

    int x1 = cx + (int)(cosf(angle) * 8);
    int y1 = cy + (int)(sinf(angle) * 8);

    int x2 = cx + (int)(cosf(angle) * 21);
    int y2 = cy + (int)(sinf(angle) * 21);

    display.drawLine(x1, y1, x2, y2, WHITE);
  }

  // Reflective rings around the center
  display.drawCircle(cx, cy, 14, WHITE);
  display.drawCircle(cx, cy, 11, WHITE);

  // Center hole
  display.fillCircle(cx, cy, 4, BLACK);
  display.drawCircle(cx, cy, 4, WHITE);

  display.display();

  // Advance rotation for the next frame
  cdAngle = (cdAngle + 15) % 360;
}

// ------------------------------------------------
// SHOW COMMAND ON OLED
// ------------------------------------------------
void showCommand(const char* message) {
  strncpy(currentMessage, message, sizeof(currentMessage) - 1);
  currentMessage[sizeof(currentMessage) - 1] = '\0';

  showingMessage = true;
  messageStartTime = millis();

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  int16_t x1, y1;
  uint16_t textWidth, textHeight;

  display.getTextBounds(
    currentMessage,
    0, 0,
    &x1, &y1,
    &textWidth, &textHeight
  );

  int x = (SCREEN_WIDTH - textWidth) / 2;
  int y = (SCREEN_HEIGHT - textHeight) / 2;

  display.setCursor(x, y);
  display.print(currentMessage);
  display.display();
}

// ------------------------------------------------
// SEND BLUETOOTH MEDIA COMMAND
// ------------------------------------------------
void sendCommand(const char* command) {
  if (bleKeyboard.isConnected()) {
    if (strcmp(command, "PLAY / PAUSE") == 0) {
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
    }
    else if (strcmp(command, "NEXT TRACK") == 0) {
      bleKeyboard.write(KEY_MEDIA_NEXT_TRACK);
    }
    else if (strcmp(command, "PREVIOUS TRACK") == 0) {
      bleKeyboard.write(KEY_MEDIA_PREVIOUS_TRACK);
    }
  }

  // Display the command even if Bluetooth is disconnected
  showCommand(command);
}

// ------------------------------------------------
// SETUP
// ------------------------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT);

  // ESP32 I2C pins
  Wire.begin(21, 22);

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not detected!");

    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.display();

  // Start Bluetooth keyboard
  bleKeyboard.begin();

  // Initialize touch state
  lastReading = digitalRead(TOUCH_PIN);
  stableState = lastReading;
  touchPressed = (stableState == TOUCH_ACTIVE);

  if (touchPressed) {
    pressStartTime = millis();
  }

  // Draw the first CD frame
  drawCD();
  lastAnimationTime = millis();

  Serial.println("Desk Buddy CD Player started");
}

// ------------------------------------------------
// MAIN LOOP
// ------------------------------------------------
void loop() {
  unsigned long now = millis();

  // ----- Read and debounce touch sensor -----
  bool reading = digitalRead(TOUCH_PIN);

  if (reading != lastReading) {
    lastReading = reading;
    lastChangeTime = now;
  }

  if (now - lastChangeTime >= DEBOUNCE_MS) {
    if (stableState != reading) {
      stableState = reading;

      // Touch started
      if (stableState == TOUCH_ACTIVE) {
        touchPressed = true;
        pressStartTime = now;
        longPressHandled = false;
      }

      // Touch released
      else {
        touchPressed = false;

        if (!longPressHandled) {
          // Second tap = next track
          if (waitingForSecondTap &&
              now - firstTapTime <= DOUBLE_CLICK_MS) {

            waitingForSecondTap = false;
            sendCommand("NEXT TRACK");
          }

          // First tap: wait for a possible second tap
          else {
            waitingForSecondTap = true;
            firstTapTime = now;
          }
        }
      }
    }
  }

  // ----- Long press = previous track -----
  if (touchPressed &&
      !longPressHandled &&
      now - pressStartTime >= LONG_PRESS_MS) {

    longPressHandled = true;
    waitingForSecondTap = false;

    sendCommand("PREVIOUS TRACK");
  }

  // ----- Single tap = play / pause -----
  if (waitingForSecondTap &&
      !touchPressed &&
      now - firstTapTime > DOUBLE_CLICK_MS) {

    waitingForSecondTap = false;
    sendCommand("PLAY / PAUSE");
  }

  // ----- Return to CD after command message -----
  if (showingMessage &&
      now - messageStartTime >= MESSAGE_MS) {

    showingMessage = false;
    lastAnimationTime = now;
    drawCD();
  }

  // ----- Animate the CD continuously -----
  if (!showingMessage &&
      now - lastAnimationTime >= FRAME_INTERVAL_MS) {

    lastAnimationTime = now;
    drawCD();
  }

  // Short delay to reduce unnecessary CPU usage
  delay(2);
}
