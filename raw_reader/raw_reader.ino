#include "HX711.h"

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN  = 3;

HX711 scale;

// calibration constants
const float m = -0.0013;
const float b = -4237.73 + 1527 + 247.4 + 7.5;

#define WINDOW_SIZE 5
float readings[WINDOW_SIZE];
int readIndex = 0;
float total = 0;
float avg = 0;

void setup() {
  Serial.begin(230400);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);
  scale.tare();

  for (int i = 0; i < WINDOW_SIZE; i++) {
    readings[i] = 0;
  }
}

void loop() {
  if (scale.is_ready()) {
    long reading = scale.read();        
    float grams = m * reading + b;

    // moving average
    total -= readings[readIndex];          // remove oldest value
    readings[readIndex] = grams;           // store new reading
    total += grams;                        // add new value
    readIndex = (readIndex + 1) % WINDOW_SIZE;  // advance circular index
    avg = total / WINDOW_SIZE;

    Serial.print(reading);
    Serial.print('\t');
    Serial.print(grams, 2);
    Serial.print('\t');
    Serial.println(avg, 2);
  }
}
