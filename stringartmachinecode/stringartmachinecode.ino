#include <Adafruit_NeoPixel.h>

#define LED_PIN     5
#define NUM_LEDS    300
#define STEP_PIN    2
#define DIR_PIN     4

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int colors[][4] = {
  {255, 0, 0, 20},
  {0, 255, 0, 30},
  {0, 0, 255, 40},{0, 0, 255, 41},{0, 0, 255, 51},{0, 0, 255, 63},{0, 0, 255, 7},{0, 0, 255, 81},{0, 0, 255, 40},{0, 0, 255, 90},{0, 0, 255, 140},
  {-1, -1, -1, -1}
};

// Motor settings
const int stepsPerRevolution = 200;
const int microstep = 16;
const int totalSteps = stepsPerRevolution * microstep; // 3200

// Wheel divided into 300 segments
const int totalSegments = 300;

// Current wheel position
int currentSegment = 0;

void setup() {
  strip.begin();
  strip.show();

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
}

void loop() {

  int i = 0;

  while (true) {

    int r = colors[i][0];
    int g = colors[i][1];
    int b = colors[i][2];
    int targetSegment = colors[i][3];

    if (r == -1 && g == -1 && b == -1 && targetSegment == -1)
      break;

    setAllLEDs(r, g, b);

    rotateToSegment(targetSegment);

    delay(2000);

    i++;
  }

  setAllLEDs(0, 0, 0);

  while (1);
}

void setAllLEDs(int r, int g, int b) {

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }

  strip.show();
}

void rotateToSegment(int targetSegment) {

  // Difference between current and target
  int segmentDifference = targetSegment - currentSegment;

  // Convert segments to motor steps
  float stepsPerSegment = (float)totalSteps / totalSegments;

  int stepsToMove = round(segmentDifference * stepsPerSegment);

  if (stepsToMove >= 0) {
    digitalWrite(DIR_PIN, HIGH);
  } else {
    digitalWrite(DIR_PIN, LOW);
    stepsToMove = abs(stepsToMove);
  }

  for (int s = 0; s < stepsToMove; s++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(800);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(800);
  }

  currentSegment = targetSegment;
}

