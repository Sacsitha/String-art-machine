#include <Adafruit_NeoPixel.h>

#define LED_PIN  5        // Change to your data pin
#define NUM_LEDS 300      // Number of LEDs in your strip

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Your color array: [R, G, B, delay placeholder]
int colors[][4] = {
  {255, 0, 0, 20},   // Red
  {0, 255, 0, 0},    // Green
  {-1, -1, -1, -1}   // Stop signal
};

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  int i = 0;
  while (true) {
    int r = colors[i][0];
    int g = colors[i][1];
    int b = colors[i][2];

    // Check for stop condition
    if (r == -1 && g == -1 && b == -1) {
      break;
    }

    // Set color to all LEDs
    setAllLEDs(r, g, b);
    delay(2000);  // Light for 2 seconds
    i++;
  }

  // After finishing, turn off LEDs
  setAllLEDs(0, 0, 0);
  while (1);  // Stop loop permanently
}

void setAllLEDs(int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}
