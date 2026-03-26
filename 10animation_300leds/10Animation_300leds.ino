#include <FastLED.h>

#define LED_PIN     5
#define NUM_LEDS    300
#define BRIGHTNESS  150
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

uint8_t effect = 0;
unsigned long lastSwitch = 0;
const unsigned long effectTime = 7000;

// ---------- SETUP ----------
void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  randomSeed(analogRead(0));
}

// ---------- LOOP ----------
void loop() {
  if (millis() - lastSwitch > effectTime) {
    effect = (effect + 1) % 20;
    lastSwitch = millis();
    FastLED.clear();
  }

  switch (effect) {
    case 0: runningDots(); break;
    case 1: rainbowWave(); break;
    case 2: confetti(); break;
    case 3: meteorRain(); break;
    case 4: centerBurst(); break;
    case 5: colorChase(); break;
    case 6: breathingRainbow(); break;
    case 7: fireEffect(); break;
    case 8: sparkleFlash(); break;
    case 9: sinelon(); break;

    case 10: dualColorScan(); break;
    case 11: plasma(); break;
    case 12: randomBlocks(); break;
    case 13: bouncingBalls(); break;
    case 14: rainbowScroll(); break;
    case 15: lightning(); break;
    case 16: twinkleStars(); break;
    case 17: gradientFlow(); break;
    case 18: theaterChaseRainbow(); break;
    case 19: rippleEffect(); break;
  }
}

// =====================================================
// 0️⃣ Running Multi-Color Dots
void runningDots() {
  static int pos = 0;
  static uint8_t hue = 0;

  FastLED.clear();
  for (int i = 0; i < 4; i++)
    leds[(pos + i) % NUM_LEDS] = CHSV(hue, 255, 255);

  FastLED.show();
  pos++;
  hue += 4;
  delay(20);
}

// 1️⃣ Rainbow Wave
void rainbowWave() {
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CHSV(hue + i * 2, 255, 255);
  FastLED.show();
  hue++;
  delay(20);
}

// 2️⃣ Confetti
void confetti() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = random(NUM_LEDS);
  leds[pos] += CHSV(random8(), 200, 255);
  FastLED.show();
  delay(15);
}

// 3️⃣ Meteor Rain
void meteorRain() {
  static int pos = 0;
  static uint8_t hue = 0;

  fadeToBlackBy(leds, NUM_LEDS, 40);
  leds[pos] = CHSV(hue, 255, 255);

  FastLED.show();
  pos = (pos + 1) % NUM_LEDS;
  hue += 3;
  delay(20);
}

// 4️⃣ Center Burst
void centerBurst() {
  static int step = 0;
  int c = NUM_LEDS / 2;

  FastLED.clear();
  for (int i = 0; i < step; i++) {
    if (c + i < NUM_LEDS) leds[c + i] = CHSV(i * 5, 255, 255);
    if (c - i >= 0) leds[c - i] = CHSV(i * 5, 255, 255);
  }

  FastLED.show();
  step++;
  if (step > c) step = 0;
  delay(20);
}

// 5️⃣ Color Chase
void colorChase() {
  static int pos = 0;
  static uint8_t hue = 0;

  FastLED.clear();
  leds[pos] = CHSV(hue, 255, 255);
  FastLED.show();

  pos = (pos + 1) % NUM_LEDS;
  hue += 10;
  delay(25);
}

// 6️⃣ Breathing Rainbow
void breathingRainbow() {
  static uint8_t bright = 0;
  static int dir = 1;
  static uint8_t hue = 0;

  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, bright));
  FastLED.show();

  bright += dir * 3;
  if (bright == 0 || bright >= 200) dir = -dir;
  hue++;
  delay(20);
}

// 7️⃣ Fire Effect
void fireEffect() {
  static byte heat[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++)
    heat[i] = qsub8(heat[i], random8(0, 40));

  for (int i = NUM_LEDS - 1; i >= 2; i--)
    heat[i] = (heat[i - 1] + heat[i - 2] + heat[i - 2]) / 3;

  if (random8() < 120)
    heat[random8(5)] = random8(160, 255);

  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = HeatColor(heat[i]);

  FastLED.show();
  delay(20);
}

// 8️⃣ Sparkle Flash
void sparkleFlash() {
  FastLED.clear();
  for (int i = 0; i < 10; i++)
    leds[random(NUM_LEDS)] = CHSV(random8(), 255, 255);
  FastLED.show();
  delay(80);
}

// 9️⃣ Sinelon
void sinelon() {
  fadeToBlackBy(leds, NUM_LEDS, 30);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(millis() / 10, 255, 255);
  FastLED.show();
  delay(20);
}

// =====================================================
// 🔟 Dual Color Scan
void dualColorScan() {
  static int pos = 0;
  FastLED.clear();
  leds[pos] = CRGB::Blue;
  leds[NUM_LEDS - pos - 1] = CRGB::Red;
  FastLED.show();
  pos = (pos + 1) % NUM_LEDS;
  delay(20);
}

// 1️⃣1️⃣ Plasma
void plasma() {
  static uint16_t t = 0;
  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] = CHSV(sin8(i * 5 + t), 255, 255);
  FastLED.show();
  t++;
  delay(20);
}

// 1️⃣2️⃣ Random Color Blocks
void randomBlocks() {
  FastLED.clear();
  for (int i = 0; i < NUM_LEDS; i += 20) {
    CRGB c = CHSV(random8(), 255, 255);
    for (int j = i; j < i + 20 && j < NUM_LEDS; j++)
      leds[j] = c;
  }
  FastLED.show();
  delay(400);
}

// 1️⃣3️⃣ Bouncing Balls
void bouncingBalls() {
  fadeToBlackBy(leds, NUM_LEDS, 40);
  for (int i = 0; i < 5; i++) {
    int pos = beatsin16(10 + i * 2, 0, NUM_LEDS - 1);
    leds[pos] = CHSV(i * 50, 255, 255);
  }
  FastLED.show();
  delay(20);
}

// 1️⃣4️⃣ Rainbow Scroll
void rainbowScroll() {
  static uint8_t hue = 0;
  fill_rainbow(leds, NUM_LEDS, hue, 5);
  FastLED.show();
  hue++;
  delay(20);
}

// 1️⃣5️⃣ Lightning
void lightning() {
  FastLED.clear();
  if (random8() < 40) {
    for (int i = 0; i < NUM_LEDS; i++)
      leds[i] = CRGB::White;
    FastLED.show();
    delay(30);
  }
  delay(80);
}

// 1️⃣6️⃣ Twinkle Stars
void twinkleStars() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  leds[random(NUM_LEDS)] = CHSV(random8(), 200, 255);
  FastLED.show();
  delay(40);
}

// 1️⃣7️⃣ Gradient Flow
void gradientFlow() {
  static uint8_t startHue = 0;
  fill_gradient(leds, NUM_LEDS, CHSV(startHue,255,255), CHSV(startHue+100,255,255));
  FastLED.show();
  startHue++;
  delay(30);
}

// 1️⃣8️⃣ Theater Chase Rainbow
void theaterChaseRainbow() {
  static uint8_t q = 0;
  static uint8_t hue = 0;
  FastLED.clear();
  for (int i = q; i < NUM_LEDS; i += 3)
    leds[i] = CHSV(hue, 255, 255);
  FastLED.show();
  q = (q + 1) % 3;
  hue++;
  delay(60);
}

// 1️⃣9️⃣ Ripple Effect
void rippleEffect() {
  static int center = random(NUM_LEDS);
  static int step = 0;

  fadeToBlackBy(leds, NUM_LEDS, 30);
  if (center + step < NUM_LEDS)
    leds[center + step] = CHSV(step * 8, 255, 255);
  if (center - step >= 0)
    leds[center - step] = CHSV(step * 8, 255, 255);

  FastLED.show();
  step++;
  if (step > 40) {
    step = 0;
    center = random(NUM_LEDS);
  }
  delay(20);
}
