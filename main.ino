#include "HX711.h"

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {
  if (scale.is_ready()) {
    long reading = scale.read_average(10);  // raw ADC value
    Serial.println(reading);
  } else {
    Serial.println("HX711 not found");
  }
  delay(100);
}
