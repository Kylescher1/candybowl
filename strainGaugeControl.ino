#include <Servo.h>
#include "HX711.h"

// Strain Gauge Pins
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN  = 3;


HX711 scale;

// Servo Info
Servo myservo;
const int SERVO_PIN = 6;

// Calibration Constants
const float m = -0.0013;
const float b = -4237.73 + 1527 + 247.4 + 7.5;

// Moving Average?
#define WINDOW_SIZE 5
float readings[WINDOW_SIZE];
int readIndex = 0;
float total = 0;
float avg = 0;

// Control Information 
int threshold = 85; // grams
int weights [2];
int cycles = 0;
double weightChange;
bool run = true;

void setup() 
{
  Serial.begin(230400);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);
  scale.tare();

  for (int i = 0; i < WINDOW_SIZE; i++) {
    readings[i] = 0;
  }
}

void loop() 
{
  if (run)
  {
  if (scale.is_ready()) 
  {
    long reading = scale.read();        
    float grams = m * reading + b;

    weights[cycles%2] = grams; // 0 is prev, 1 is current

  
    Serial.print(threshold);
    Serial.print(" ' ");
    Serial.println(weightChange);

    if(cycles != 0)
      {
        weightChange = weights[0] - weights[1];
        if(weightChange > threshold)
      {
        myservo.attach(SERVO_PIN,1,20);
        run = false;
      }
        weights[0] = weights[1];
    }
    cycles++;
  }
  }
}
  
