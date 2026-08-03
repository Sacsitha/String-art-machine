#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include "monalisa_data.h"

#define LED_PIN     2
#define NUM_LEDS    174
#define HALL_PIN 13 
#define SERVO_PIN 27
#define STEP_PIN 12
#define DIR_PIN 14

//Stepper Motor 
const int stepsPerRevolution = 200;
const int microstep = 1;
const int totalSteps = stepsPerRevolution * microstep*4;
//const int totalSteps = 3200;
const int totalSegments = 300;

int currentSegment = 0;
const int startDelay = 20000;
const int runDelay = 18000;
const int accelerationStep = 1;
//Server Motor movement
Servo armServo;
const int ARM_DOWN = 115;
const int ARM_UP = 90;
const int ARM_FORWARD = 80;
//LED format
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
void setup()
{
  Serial.begin(115200);

  strip.begin();
  strip.show();

  pinMode(HALL_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  armServo.attach(SERVO_PIN);
  armServo.write(ARM_DOWN);

  homeMachine();
}
/*void setup() {
  //Initialize LED Strip
  strip.begin();
  strip.show();
  //Configure pins
  pinMode(HALL_PIN, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  //Initialize servo
  armServo.attach(SERVO_PIN);
  armServo.write(ARM_DOWN);
  //Find the starting position using hall effect sensor
  homeMachine(); 
}
*/
void loop() {

  int i = 0;

  int prevR = -1;
  int prevG = -1;
  int prevB = -1;

  while (true) {
    //Reads the data from "monalisa_data.h"
    int r = colors[i][0];
    int g = colors[i][1];
    int b = colors[i][2];
    int targetSegment = colors[i][3];
    //End of data
    if (r == -1 && g == -1 && b == -1 && targetSegment == -1)
      break;
    //detect color chae in the data
    if (i > 0 && (r != prevR || g != prevG || b != prevB)) {
      setAllLEDs(r, g, b);
      delay(180000);  
    }
    //Show the current color
    //setAllLEDs(r, g, b);
    lightNearestLED(targetSegment, r, g, b);
    delay(500);
    //Move the wheel(Stepper Motor)
    rotateToSegment(targetSegment);
    delay(500);
    //Move the arm(Server Motor)
    moveArm();

    delay(500);
    prevR = r;
    prevG = g;
    prevB = b;
    i++;
  }
  strip.clear();
strip.show();
  //Reset the LED Strip
  //setAllLEDs(0, 0, 0);
  while (1); 
}
/*void loop()
{
  rotateToSegment(106);

  while (1);
}*/
//LED color changing function
void setAllLEDs(int r, int g, int b) {

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }

  strip.show();
}
void moveArm() {
  //Lift arm to 90°
  armServo.write(ARM_UP);
  delay(500);
  //Lift arm to 110°
  armServo.write(ARM_FORWARD);
  delay(500);
  //move the wheel --Helps guide the thread
  jogMotor(HIGH, 10);
  delay(500);
  //Lower the arm to 20°
  armServo.write(ARM_DOWN);
  delay(500);
}
//A single step movement of the stepper motor --wheel
void stepMotor(int delayTime)
{
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(delayTime);

  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(delayTime);
}
//Stepper motor movement for threadin through the nail
void jogMotor(bool direction, int steps)
{
  digitalWrite(DIR_PIN, direction);

  for (int i = 0; i < steps; i++)
  {
    stepMotor(3000);
  }
}
/*void moveMotor(bool direction, int steps)
{
  digitalWrite(DIR_PIN, direction);

  // Accelerate
  int delayTime = startDelay;

  while (delayTime > runDelay)
  {
    stepMotor(delayTime);
    delayTime -= accelerationStep;
  }

  // Constant speed
  for (int i = 0; i < steps; i++)
  {
    stepMotor(runDelay);
  }

  // Decelerate
  delayTime = runDelay;

  while (delayTime < startDelay)
  {
    stepMotor(delayTime);
    delayTime += accelerationStep;
  }
}*/
void moveMotor(bool direction, int steps)
{
  digitalWrite(DIR_PIN, direction);

  for (int i = 0; i < steps; i++)
  {
    stepMotor(3000);
  }
}


void rotateToSegment(int targetSegment)
{
  int segmentDifference = targetSegment - currentSegment;

  if (segmentDifference > 150)
  {
    segmentDifference -= 300;
  }

  if (segmentDifference < -150)
  {
    segmentDifference += 300;
  }
  float stepsPerSegment = (float)totalSteps / totalSegments;

  int motorSteps = round(abs(segmentDifference) * stepsPerSegment);

  Serial.print("Current Segment: ");
  Serial.println(currentSegment);

  Serial.print("Target Segment: ");
  Serial.println(targetSegment);

  Serial.print("Motor Steps: ");
  Serial.println(motorSteps);

  if (segmentDifference >= 0)
    moveMotor(HIGH, motorSteps);
  else
    moveMotor(LOW, motorSteps);

  currentSegment = targetSegment;
}
void homeMachine()
{
  //Serial.begin(115200);

  digitalWrite(DIR_PIN, HIGH);

  // Search for magnet
  while (digitalRead(HALL_PIN) == HIGH)
  {
    stepMotor(12000);
  }

  // Move away a little
  digitalWrite(DIR_PIN, LOW);

  for(int i=0;i<50;i++)
      stepMotor(12000);

  // Slowly approach again
  digitalWrite(DIR_PIN, HIGH);

  while (digitalRead(HALL_PIN) == HIGH)
  {
    stepMotor(12000);
  }

  currentSegment = 0;

  Serial.println("Home Position Set");
  currentSegment = 0;
  setAllLEDs(255,255,51);
  delay(1000);
}

/*
void rotateToSegment(int targetSegment)
{
  int segmentDifference = targetSegment - currentSegment;

  // Choose shortest direction
  if (segmentDifference > totalSegments / 2)
  {
    segmentDifference -= totalSegments;
  }
  else if (segmentDifference < -totalSegments / 2)
  {
    segmentDifference += totalSegments;
  }

  float stepsPerSegment = (float)totalSteps / totalSegments;

  int motorSteps = round(abs(segmentDifference) * stepsPerSegment);


  Serial.print("Moving segments: ");
  Serial.println(segmentDifference);

  Serial.print("Steps: ");
  Serial.println(motorSteps);


  if (segmentDifference > 0)
  {
    moveMotor(HIGH, motorSteps);
  }
  else if (segmentDifference < 0)
  {
    moveMotor(LOW, motorSteps);
  }

  currentSegment = targetSegment;
}
*/
void lightNearestLED(int nail, int r, int g, int b)
{
    strip.clear();

    int ledIndex = round((nail * (NUM_LEDS - 1)) / (float)(totalSegments - 1));

    strip.setPixelColor(ledIndex, strip.Color(r, g, b));
    strip.show();
}

