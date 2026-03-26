#define BLYNK_TEMPLATE_ID "xxxxxxxxxxxxxxxxxxxxxxx"
#define BLYNK_TEMPLATE_NAME "ShowMessage"
#define BLYNK_AUTH_TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxx"

#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>



char ssid[] = "Rudra";
char pass[] = "Rudra@2005";

#define MATRIX_PIN 5
#define SOUND_PIN 13
#define BUTTON_PIN 14
#define ANI_PIN 12

BlynkTimer timer;
String message = "";

int y = -1;
int z = -1;
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(
  32, 8, MATRIX_PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800);


// Heart shape (8x8)
const byte heart[8][8] = {
  { 0, 1, 1, 0, 0, 1, 1, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1 },
  { 0, 1, 1, 1, 1, 1, 1, 0 },
  { 0, 0, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 0, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // SDA, SCL for ESP32
 
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    // while (1)
    //   ;
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time to compile time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  matrix.begin();
  matrix.setBrightness(60);  // Adjust brightness (0-255)
  matrix.setTextWrap(false);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(ANI_PIN, INPUT_PULLUP);
}
BLYNK_WRITE(V0) {
  message = param.asStr();
}
const int width = 32;
const int height = 8;

uint32_t dropColors[32]; // store color for each column drop
bool dropActive[32] = {false}; // track if drop is active in that column
int dropY[32] = {0}; // Y-position of each drop


//.....************************************************************************

void scrollText(String text, uint16_t color) {
  if (text == "1305") {
    heartNameAnimation();
    return;
  }
  int x = matrix.width();
  int textLength = text.length() * 6;

  while (x > -textLength) {
    matrix.fillScreen(0);
    matrix.setCursor(x, 0);
    matrix.setTextColor(color);
    matrix.print(text);
    matrix.show();
    x--;
    delay(80);
  }
}
//**************************************
int x = matrix.width();
// int count = 0;
unsigned long lastSwitchTime = 0;
int displayStage = 0;  // 0: time, 1: date, 2: day
const unsigned long displayDuration = 3000;
//**************************************************************************
void autoDateTime(DateTime now) {
  unsigned long currentMillis = millis();

  if (currentMillis - lastSwitchTime >= displayDuration) {
    lastSwitchTime = currentMillis;
    matrix.fillScreen(0);

    if (displayStage == 0) {
      // Show time
      String hourStr = (now.hour() < 10 ? "0" : "") + String(now.hour(), DEC) + ":" + (now.minute() < 10 ? "0" : "") + String(now.minute(), DEC);
      matrix.setCursor(1, 0);
      matrix.setTextColor(matrix.Color((random(256) + 200) % 256, (random(256) + 150) % 256, (random(256) + 100) % 256));
      matrix.print(hourStr);
    } else if (displayStage == 1) {
      // Show date
      String dayStr = (now.day() < 10 ? "0" : "") + String(now.day(), DEC) + "/" + (now.month() < 10 ? "0" : "") + String(now.month(), DEC);
      matrix.setCursor(2, 0);
      matrix.setTextColor(matrix.Color((random(256) + 150) % 256, (random(256) + 150) % 256, (random(256) + 200) % 256));
      matrix.print(dayStr);
    } else if (displayStage == 2) {
      // Show day
      String dayOfWeek = daysOfTheWeek[now.dayOfTheWeek()];
      matrix.setCursor(9, 0);
      matrix.setTextColor(matrix.Color((random(256) + 200) % 256, (random(256) + 150) % 256, (random(256) + 100) % 256));
      matrix.print(dayOfWeek);
    } else if (displayStage == 3) {
      String temp = String(rtc.getTemperature()) + "°C";
      matrix.setCursor(1, 0);
      matrix.setTextColor(matrix.Color(255, 105, 180));
      matrix.print(temp);
    }
    matrix.show();
    displayStage = (displayStage + 1) % 4;
  }
}
//*********************************************************************
void rainDropAnimation(unsigned long duration_ms) {
  unsigned long start = millis();

  while (millis() - start < duration_ms) {
    matrix.fillScreen(0);

    // Launch new drops randomly
    for (int x = 0; x < width; x++) {
      if (!dropActive[x] && random(0, 100) < 8) {  // 8% chance to start
        dropActive[x] = true;
        dropY[x] = 0;
        dropColors[x] = matrix.Color(random(256), random(256), random(256));
      }
    }

    // Move and draw active drops
    for (int x = 0; x < width; x++) {
      if (dropActive[x]) {
        matrix.drawPixel(x, dropY[x], dropColors[x]);
        dropY[x]++;
        if (dropY[x] >= height) {
          dropActive[x] = false;  // reset drop
        }
      }
    }

    matrix.show();
    delay(60);
  }
}
//*************************
  void spiralFill() {
    int width = matrix.width();
    int height = matrix.height();
    int left = 0, right = width - 1;
    int top = 0, bottom = height - 1;

    matrix.fillScreen(0);

    // Column spiral (vertical pass): same color per column
    while (left <= right && top <= bottom) {
      uint16_t colColor = matrix.Color(random(256), random(256), random(256));
      for (int y = top; y <= bottom; y++) {
        matrix.drawPixel(left, y, colColor);
        matrix.show();
        delay(10);
      }
      left++;
    }

    // Reset for row spiral (horizontal pass): same color per row
    left = 0;
    top = 0;
    while (top <= bottom && left <= right) {
      uint16_t rowColor = matrix.Color(random(256), random(256), random(256));
      for (int x = left; x <= right; x++) {
        matrix.drawPixel(x, top, rowColor);
        matrix.show();
        delay(20);
      }
      top++;
    }

    // Hold the final display
    matrix.fillScreen(0);
    matrix.show();
  }
  //.........................................
  void animation() {

    int x = random(8);

    while (x == y) {
      x = random(8);
    }
    y = x;

    const int totalPixels = matrix.width() * matrix.height();
    bool pixelLit[totalPixels] = { false };
    int litCount = 0;
    switch (x) {
      case 0:
        for (int x = 0; x < matrix.width(); x++) {
          for (int y = 0; y < matrix.height(); y++) {
            matrix.drawPixel(x, y, matrix.Color(random(256), random(256), random(256)));  // Blue line effect
          }
          matrix.show();
          delay(50);
        }
        delay(1500);
        break;
      case 1:
        while (litCount < totalPixels) {
          int index = random(totalPixels);
          if (!pixelLit[index]) {
            int x = index % matrix.width();
            int y = index / matrix.width();
            matrix.drawPixel(x, y, matrix.Color(255, 50, 100));
            matrix.show();
            pixelLit[index] = true;
            litCount++;
            delay(30);  // Adjust speed of fill
          }
        }

        delay(1500);  // Keep final display for a moment
        break;
      case 2:
        for (int y = 0; y < matrix.height(); y++) {
          for (int x = 0; x < matrix.width(); x++) {
            matrix.drawPixel(x, y, matrix.Color(random(256), random(256), random(256)));
          }
          matrix.show();
          delay(200);
        }

        delay(800);  // Pause after full display

        // Optional: Clear row by row in reverse
        for (int y = matrix.height() - 1; y >= 0; y--) {
          for (int x = 0; x < matrix.width(); x++) {
            matrix.drawPixel(x, y, 0);  // Turn off pixel
          }
          matrix.show();
          delay(100);
        }
        delay(1500);
        break;
      case 3:
        colorCircle();
        break;
      case 4:
        spiralFill();
        break;
      case 5:
        sparkleAnimation();
        break;
      case 6:
        spiralFillSimultaneous();
        break;
      case 7:
        threeHeartAnimation(10000);
        break;
      case 8:
        rainDropAnimation(7000);
        break;
    }
  }
  //......................................................................................................................
  void drawHeartAt(int x_offset, int y_offset, uint16_t color) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (heart[y][x]) {
          matrix.drawPixel(x + x_offset, y + y_offset, color);
        }
      }
    }
  }

  void threeHeartAnimation(unsigned long duration_ms) {
    int positions[3] = { 0, 12, 24 };  // Fixed positions for 3 hearts
    uint16_t colors[3] = {
      matrix.Color(255, 0, 0),      // Red
      matrix.Color(135, 206, 250),  // Sky Blue
      matrix.Color(255, 50, 100)    // Pinkish
    };

    unsigned long start = millis();
    while (millis() - start < duration_ms) {
      matrix.fillScreen(0);

      // Choose one heart and one color randomly
      int indexToGlow = random(0, 3);
      uint16_t selectedColor = colors[random(0, 3)];

      // Draw all hearts with color 0 except the glowing one
      for (int i = 0; i < 3; i++) {
        if (i == indexToGlow) {
          drawHeartAt(positions[i], 0, selectedColor);
        } else {
          drawHeartAt(positions[i], 0, 0);  // OFF
        }
      }

      matrix.show();
      delay(500);  // How long one heart glows
    }
  }
  void colorCircle() {

    unsigned long startTime = millis();
    int r = 255, g = 0, b = 0;

    while (millis() - startTime < 5000) {
      while (g < 255 && millis() - startTime < 5000) {
        g += 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (r > 0 && millis() - startTime < 5000) {
        r -= 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (b < 255 && millis() - startTime < 5000) {
        b += 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (g > 0 && millis() - startTime < 5000) {
        g -= 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (r < 255 && millis() - startTime < 5000) {
        r += 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
      while (b > 0 && millis() - startTime < 5000) {
        b -= 5;
        matrix.fillScreen(matrix.Color(r, g, b));
        matrix.show();
        delay(10);
      }
    }
    matrix.fillScreen(0);
    matrix.show();
  }
  void spiralFillSimultaneous() {
    matrix.fillScreen(0);
    matrix.show();

    // Dimensions
    const int w = matrix.width();
    const int h = matrix.height();

    // Left spiral bounds
    int l_top = 0, l_bottom = h - 1;
    int l_left = 0, l_right = (w / 2) - 1;

    // Right spiral bounds
    int r_top = 0, r_bottom = h - 1;
    int r_left = w / 2, r_right = w - 1;

    // Colors
    uint16_t colorL = matrix.Color(random(256), random(256), random(256));
    uint16_t colorR = matrix.Color(random(256), random(256), random(256));

    // Storage for pixel coordinates
    std::vector<std::pair<int, int>> leftSpiral;
    std::vector<std::pair<int, int>> rightSpiral;

    // Generate left spiral coordinates
    while (l_left <= l_right && l_top <= l_bottom) {
      for (int x = l_left; x <= l_right; x++) leftSpiral.push_back({ x, l_top });
      l_top++;
      for (int y = l_top; y <= l_bottom; y++) leftSpiral.push_back({ l_right, y });
      l_right--;
      for (int x = l_right; x >= l_left; x--) leftSpiral.push_back({ x, l_bottom });
      l_bottom--;
      for (int y = l_bottom; y >= l_top; y--) leftSpiral.push_back({ l_left, y });
      l_left++;
    }

    // Generate right spiral coordinates
    while (r_left <= r_right && r_top <= r_bottom) {
      for (int x = r_left; x <= r_right; x++) rightSpiral.push_back({ x, r_top });
      r_top++;
      for (int y = r_top; y <= r_bottom; y++) rightSpiral.push_back({ r_right, y });
      r_right--;
      for (int x = r_right; x >= r_left; x--) rightSpiral.push_back({ x, r_bottom });
      r_bottom--;
      for (int y = r_bottom; y >= r_top; y--) rightSpiral.push_back({ r_left, y });
      r_left++;
    }

    // Draw both spirals simultaneously
    int maxSteps = max(leftSpiral.size(), rightSpiral.size());
    for (int i = 0; i < maxSteps; i++) {
      if (i < leftSpiral.size()) {
        int x = leftSpiral[i].first;
        int y = leftSpiral[i].second;
        matrix.drawPixel(x, y, colorL);
      }
      if (i < rightSpiral.size()) {
        int x = rightSpiral[i].first;
        int y = rightSpiral[i].second;
        matrix.drawPixel(x, y, colorR);
      }
      matrix.show();
      delay(30);
    }

    delay(1000);
    matrix.fillScreen(0);
    matrix.show();
  }
  void sparkleAnimation() {
    unsigned long startTime = millis();
    int totalPixels = matrix.width() * matrix.height();

    while (millis() - startTime < 7000) {
      // Light up a few random pixels with random colors
      for (int i = 0; i < 5; i++) {
        int x = random(matrix.width());
        int y = random(matrix.height());
        uint16_t color = matrix.Color(random(100, 256), random(50, 256), random(256));
        matrix.drawPixel(x, y, color);
      }

      matrix.show();
      delay(50);

      // Slight fade effect: randomly clear a few pixels
      for (int i = 0; i < 100; i++) {
        int x = random(matrix.width());
        int y = random(matrix.height());
        matrix.drawPixel(x, y, 0);
      }
    }

    matrix.fillScreen(0);  // Clear after animation
    matrix.show();
  }

  const uint16_t RED = matrix.Color(255, 0, 0);
  const uint16_t SKY = matrix.Color(0, 255, 255);
  const uint16_t PINK = matrix.Color(255, 50, 100);
  const uint16_t COLORS[] = { RED, SKY, PINK };

  // Draw full heart
  void drawHeart(uint16_t color, int offsetX = 12, int offsetY = 0) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (heart[y][x]) {
          matrix.drawPixel(offsetX + x, offsetY + y, color);
        }
      }
    }
  }

  // Split heart apart
  void splitHeart(uint16_t color, int step) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if (heart[y][x]) {
          int xOffset = (x < 4) ? -step : step;
          matrix.drawPixel(12 + x + xOffset, y, color);
        }
      }
    }
  }

  // Reveal each letter of name one by one
  void revealName(String name) {
    int spacing = 6;
    int startX = 2;
    for (int i = 0; i < name.length(); i++) {
      matrix.setCursor(startX + i * spacing, 0);
      matrix.setTextColor(COLORS[random(3)]);
      matrix.print(name.charAt(i));
      matrix.show();
      delay(400);
    }
  }

  // Main animation
  void heartNameAnimation() {
    matrix.fillScreen(0);
    matrix.setTextWrap(false);

    // Pulse effect
    for (int i = 0; i < 3; i++) {
      drawHeart(COLORS[i % 3]);
      matrix.show();
      delay(300);
      drawHeart(0);
      matrix.show();
      delay(200);
    }

    // Split heart
    for (int i = 0; i <= 4; i++) {
      matrix.fillScreen(0);
      splitHeart(PINK, i);
      matrix.show();
      delay(150);
    }

    delay(300);

    // Show name
    matrix.fillScreen(0);
    revealName("PRITI");

    delay(1000);

    // Close heart again
    for (int i = 4; i >= 0; i--) {
      matrix.fillScreen(0);
      splitHeart(PINK, i);
      matrix.show();
      delay(150);
    }

    drawHeart(PINK);
    matrix.show();
    delay(2000);
  }
  void animatePRITI() {
    int steps = 6;
    int frameDelay = 100;

    // Fixed positions for final alignment
    int pxFinal = 0;
    int rxFinal = 6;
    int txFinal = 18;
    int ixFinal = 24;    // Right-side 'I'
    int middleI_x = 12;  // Leave 6-pixel space between R and T

    // 1. PR comes from left, TI comes from right (with gap for center I)
    for (int step = 0; step <= steps; step++) {
      matrix.fillScreen(0);

      int px = map(step, 0, steps, -6, pxFinal);
      int rx = map(step, 0, steps, -6, rxFinal);
      int tx = map(step, 0, steps, 32, txFinal);
      int ix = map(step, 0, steps, 32, ixFinal);

      matrix.setCursor(px, 0);
      matrix.setTextColor(matrix.Color(255, 0, 0));  // Red
      matrix.print('P');

      matrix.setCursor(rx, 0);
      matrix.setTextColor(matrix.Color(255, 50, 100));  // Pinkish
      matrix.print('R');

      matrix.setCursor(tx, 0);
      matrix.setTextColor(matrix.Color(0, 255, 255));  // Sky Blue
      matrix.print('T');

      matrix.setCursor(ix, 0);
      matrix.setTextColor(matrix.Color(255, 100, 0));  // Orange
      matrix.print('I');

      matrix.show();
      delay(frameDelay);
    }

    // 2. Drop center 'I'
    for (int y = -8; y <= 0; y++) {
      matrix.fillScreen(0);

      // Fixed PR
      matrix.setCursor(pxFinal, 0);
      matrix.setTextColor(matrix.Color(255, 0, 0));
      matrix.print('P');
      matrix.setCursor(rxFinal, 0);
      matrix.setTextColor(matrix.Color(255, 50, 100));
      matrix.print('R');

      // Fixed TI
      matrix.setCursor(txFinal, 0);
      matrix.setTextColor(matrix.Color(0, 255, 255));
      matrix.print('T');
      matrix.setCursor(ixFinal, 0);
      matrix.setTextColor(matrix.Color(255, 100, 0));
      matrix.print('I');

      // Dropping center I
      matrix.setCursor(middleI_x, y);
      matrix.setTextColor(matrix.Color(255, 255, 0));  // Yellow
      matrix.print('I');

      matrix.show();
      delay(80);
    }

    delay(3000);
  }
  void showName(String name) {
    int x = random(8);
    while (x == z) {
      x = random(8);
    }
    z = x;
    int scrollX = matrix.width();
    int textWidth = name.length() * 6;  // each char ~6px wide
    int startX = -textWidth;
    switch (x) {
      case 0:

        for (int x = startX; x <= 1; x++) {
          matrix.fillScreen(0);
          matrix.setCursor(x, 0);
          matrix.setTextColor(matrix.Color(100, 50, 150));
          matrix.print(name);
          matrix.show();
          delay(100);
        }

        delay(5000);
        break;
      case 1:

        for (int i = 0; i < (name.length() * 6 + matrix.width()); i++) {
          matrix.fillScreen(0);
          matrix.setCursor(scrollX, 0);
          matrix.setTextColor(matrix.Color(255, 50, 100));
          matrix.print(name);
          matrix.show();
          scrollX--;
          if (scrollX == 1) {
            delay(5000);
            return;
          }
          delay(100);
        }
        break;
      case 2:
        for (int i = 0; i < name.length(); i++) {
          matrix.fillScreen(0);            // Clear matrix before showing each letter
          matrix.setCursor(1 + i * 6, 0);  // Center position (adjust as per need)
          matrix.setTextColor(matrix.Color(random(256), (random(256) + 100) % 256, random(256)));
          matrix.print(name.charAt(i));  // Show one letter at a time
          matrix.show();
          delay(600);  // Delay before next letter
        }

        // Optional: show full name at the end
        matrix.fillScreen(0);
        matrix.setCursor(1, 0);
        matrix.setTextColor(matrix.Color((random(256) + 150) % 256, (random(256) + 100) % 256, (random(256) + 100) % 256));
        matrix.print("PRITI");
        matrix.show();
        delay(5000);
        break;
      case 3:
        showCharactersRandomly(name);
        break;
      case 4:
        waveColorLetters(name);
        break;
      case 5:
        animatePRITI();
        break;
      case 6:
        heartNameAnimation();
        break;
      case 7:
        rainDropAnimationName(7000);
        break;
    }
  }
  void rainDropAnimationName(unsigned long duration_ms) {
  unsigned long start = millis();

  while (millis() - start < duration_ms) {
    matrix.fillScreen(0);
  matrix.fillScreen(0);
  matrix.setCursor(2, 0);
  matrix.setTextColor(matrix.Color(255, 164, 30));  // Purple-pink
  matrix.print("PRITI");
  matrix.show();
    // Launch new drops randomly
    for (int x = 0; x < width; x++) {
      if (!dropActive[x] && random(0, 100) < 8) {  // 8% chance to start
        dropActive[x] = true;
        dropY[x] = 0;
        dropColors[x] = matrix.Color(random(256), random(256), random(256));
      }
    }

    // Move and draw active drops
    for (int x = 0; x < width; x++) {
      if (dropActive[x]) {
        matrix.drawPixel(x, dropY[x], dropColors[x]);
        dropY[x]++;
        if (dropY[x] >= height) {
          dropActive[x] = false;  // reset drop
        }
      }
    }

    matrix.show();
    delay(60);
  }

  matrix.fillScreen(0);
  matrix.show();
}
  void waveColorLetters(String name) {
    unsigned long duration_ms = 5000;
    int brightness = 60;
    int letterSpacing = 6;
    uint16_t baseHue = 0;
    unsigned long startTime = millis();

    matrix.setBrightness(brightness);
    matrix.setTextWrap(false);
    matrix.setTextSize(1);  // Ensure default 5x7 font

    while (millis() - startTime < duration_ms) {
      matrix.fillScreen(0);

      for (int i = 0; i < name.length(); i++) {
        int x = 1 + i * letterSpacing;
        float wave = sin((millis() / 150.0) + i);
        int y = 1 + int(wave * 2);  // Range: 0 to 3

        uint16_t color = matrix.ColorHSV((baseHue + i * 5000) % 65536, 255, 255);

        matrix.setCursor(x, y);
        matrix.setTextColor(color);
        matrix.print(name.charAt(i));
      }

      matrix.show();
      baseHue = (baseHue + 200) % 65536;
      delay(30);
    }
  }

  void showCharactersRandomly(String name) {
    int charCount = name.length();
    int xStart = 1;
    int charSpacing = 6;

    bool shown[charCount] = { false };  // Track which letters are already shown
    uint16_t colors[charCount];         // Store color per character

    unsigned long startTime = millis();

    matrix.fillScreen(0);  // Start with a blank screen

    while (millis() - startTime < 5000) {
      // Choose a random index that hasn't been shown yet
      int remaining = 0;
      for (int i = 0; i < charCount; i++) {
        if (!shown[i]) remaining++;
      }

      if (remaining == 0) break;

      int rIndex = random(remaining);  // pick one among remaining
      int chosen = -1;
      for (int i = 0; i < charCount; i++) {
        if (!shown[i]) {
          if (rIndex == 0) {
            chosen = i;
            break;
          }
          rIndex--;
        }
      }

      if (chosen >= 0) {
        shown[chosen] = true;
        colors[chosen] = matrix.Color((random(256) + 150) % 256, random(256), random(256));

        // Draw the character at its position
        int x = xStart + chosen * charSpacing;
        matrix.setCursor(x, 0);
        matrix.setTextColor(colors[chosen]);
        matrix.print(name.charAt(chosen));
        matrix.show();

        delay(400);  // slight pause before next
      }
    }

    // Hold final state for the remaining time if 7s not used up
    while (millis() - startTime < 5000) {
      delay(50);
    }

    // Optional: turn off display after 7s
    matrix.fillScreen(0);
    matrix.show();
  }

  //.................................................................................................................................

  void loop() {
    Blynk.run();
    timer.run();

    if (message.length() > 0) {
      scrollText(message, matrix.Color((random(256) + 100) % 256, (random(256) + 120) % 256, (random(256) + 140) % 256));  // Red
      Blynk.virtualWrite(V0, "");                                                                                          // Clear input field
      message = "";                                                                                                        // Clear local message
    }
    if (digitalRead(SOUND_PIN) == HIGH) {
      scrollText("BE SILENT", matrix.Color(255, 111, 97));
    }
    DateTime now = rtc.now();
    // String yearStr = String(now.year(), DEC);
    while (now.hour() == 7 && now.minute() == 0) {
      now = rtc.now();
      matrix.fillScreen(0);
      matrix.setCursor(x, 0);
      matrix.print("GOOD MORNING");
      if (--x < -66) {
        x = matrix.width();
        matrix.setTextColor(matrix.Color(random(256), random(256), random(256)));
      }
      matrix.show();
      delay(100);
    }
    while (now.hour() == 12 && now.minute() == 0) {
      now = rtc.now();
      matrix.fillScreen(0);
      matrix.setCursor(x, 0);
      matrix.print("GOOD NOON");
      if (--x < -70) {
        x = matrix.width();
        matrix.setTextColor(matrix.Color(random(256), random(256), random(256)));
      }
      matrix.show();
      delay(100);
    }
    while (now.hour() == 22 && now.minute() == 0) {
      now = rtc.now();
      matrix.fillScreen(0);
      matrix.setCursor(x, 0);
      matrix.print("GOOD NIGHT");
      if (--x < -70) {
        x = matrix.width();
        matrix.setTextColor(matrix.Color(random(256), random(256), random(256)));
      }
      matrix.show();
      delay(100);
    }
    while (now.day() == 13 && now.month() == 5 && now.hour() == 0 && now.minute() == 0) {
      now = rtc.now();
      matrix.fillScreen(0);
      matrix.setCursor(x, 0);
      matrix.print("HAPPY BIRTHDAY ");
      if (--x < -90) {
        x = matrix.width();
        matrix.setTextColor(matrix.Color(random(256), random(256), random(256)));
      }
      matrix.show();
      delay(100);
    }
    while (now.day() == 20 && now.month() == 6 && now.hour() == 0 && now.minute() == 0) {
      now = rtc.now();
      matrix.fillScreen(0);
      matrix.setCursor(x, 0);
      matrix.print("HAPPY ANNIVERSARY");
      if (--x < -110) {
        x = matrix.width();
        matrix.setTextColor(matrix.Color(random(256), random(256), random(256)));
      }
      matrix.show();
      delay(80);
    }
    autoDateTime(now);

    if (digitalRead(BUTTON_PIN) == LOW) {
      showName("PRITI");
      return;
    }
    if (digitalRead(ANI_PIN) == LOW) {
      animation();
      return;
    }
  }
  // ************************************************************************************************************
  
void wavyScroll(const String& text) {
  // Check if the text is empty
  if (text.length() == 0) return;

  matrix.fillScreen(0); // Clear the matrix each frame

  // Calculate the total pixel width of the scrollable text
  // Each default character is 5px wide + 1px gap = 6px total
  int textWidth = text.length() * 6;
int xPos=32;
  matrix.setCursor(xPos, 0);

  // Iterate through each character of the scrollable text
  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);

    int charX = xPos + (i * 6);

    // Calculate the color and vertical displacement based on a sine wave (WAVY EFFECT)
    float waveFrequency = 0.5;
    float timeFactor = millis() / 40.0;
    float sineValue = sin(charX * waveFrequency + timeFactor);

    // Map the sine wave to a Hue value (0-255)
    uint8_t charHue = (sineValue + 1.0) * 128; 

    // Convert the calculated Hue to an RGB color
    uint32_t charColor = matrix.ColorHSV(charHue * 256);

    // Apply the vertical displacement (if desired - commented out for a flatter scroll)
    // matrix.setCursor(charX, 0 + (int)(sineValue * 2)); 

    // Set the text color and draw the character
    matrix.setTextColor(charColor);
    matrix.print(c);
  }

  // Update the scroll position and loop
  xPos--;
  if (xPos < -textWidth) {
    xPos = width; // Reset to the right edge
  }
}