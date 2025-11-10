#include <Adafruit_NeoPixel.h>

#define LED_PIN     5       // WS2812B data pin
#define NUM_LEDS    300     // Total LEDs
#define STEP_PIN    2       // A4988 STEP
#define DIR_PIN     4       // A4988 DIR

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Your color + delay array
int colors[][4] = {
  {255, 0, 0, 20},    // Red
  {0, 255, 0, 0},     // Green
  {0, 0, 255, 0},     // Blue
  {-1, -1, -1, -1}    // Stop condition
};

// Motor setup
const int stepsPerRevolution = 200;  // Usually 200 for NEMA17 (1.8° per step)
const int microstep = 16;            // Adjust if using MS1, MS2, MS3 pins
const int totalSteps = stepsPerRevolution * microstep; // Full 360°

void setup() {
  strip.begin();
  strip.show();

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(DIR_PIN, HIGH);  // Set rotation direction
}

void loop() {
  int i = 0;

  while (true) {
    int r = colors[i][0];
    int g = colors[i][1];
    int b = colors[i][2];

    // Stop condition
    if (r == -1 && g == -1 && b == -1) {
      break;
    }

    // 1. Light all LEDs to given color
    setAllLEDs(r, g, b);

    // 2. Rotate motor one full turn
    rotateMotor360();

    // 3. Keep LEDs on for 2 seconds
    delay(2000);

    i++;
  }

  // Turn off LEDs at end
  setAllLEDs(0, 0, 0);

  while (1); // Stop everything
}

void setAllLEDs(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

void rotateMotor360() {
  digitalWrite(DIR_PIN, HIGH); // Choose rotation direction

  for (int s = 0; s < totalSteps; s++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(800); // Adjust for speed
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(800);
  }
}

