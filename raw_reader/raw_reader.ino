#include "HX711.h"

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN  = 3;

HX711 scale;

// calibration constants
const float m = -0.0013;
const float b = -4237.73 + 1527 + 247.4 + 7.5 + 20;

void setup() {
  Serial.begin(230400);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);
  scale.tare();
}

void loop() {
  if (scale.is_ready()) {
    long reading = scale.read();        
    float grams = m * reading + b;

    Serial.print(reading);
    Serial.print('\t');
    Serial.println(grams, 2);
  }
}
