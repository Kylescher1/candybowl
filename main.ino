#include "HX711.h"

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;

enum CalState {
  IDLE,
  CAL_WAIT_CLEAR,
  CAL_WAIT_PLACE1,
  CAL_WAIT_REMOVE1,
  CAL_WAIT_PLACE2,
  CAL_WAIT_REMOVE2,
  CAL_WAIT_PLACE3,
  CAL_INPUT_WEIGHT
};

CalState calState = IDLE;
long calSamples[3] = {0, 0, 0};
String inputLine = "";
float scaleFactor = 0.0;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("Ready. Commands: T=tare, C=calibrate");
}

void loop() {
  if (!scale.is_ready()) {
    Serial.println("HX711 not found");
    delay(250);
    return;
  }

  // Read serial input
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      String line = inputLine;
      inputLine = "";

      if (calState == CAL_INPUT_WEIGHT) {
        float grams = line.toFloat();
        if (grams > 0.0f) {
          long avgDelta = (calSamples[0] + calSamples[1] + calSamples[2]) / 3L;
          scaleFactor = (float)avgDelta / grams;
          scale.set_scale(scaleFactor);
          Serial.print("Calibration set. scaleFactor=");
          Serial.println(scaleFactor, 6);
          calState = IDLE;
        } else {
          Serial.println("Enter a positive number in grams:");
        }
      } else {
        switch (calState) {
          case CAL_WAIT_CLEAR:
            scale.tare();
            Serial.println("Zero set. Place known weight (#1) then press Enter.");
            calState = CAL_WAIT_PLACE1;
            break;

          case CAL_WAIT_PLACE1:
            calSamples[0] = scale.read_average(10) - scale.get_offset();
            Serial.println("Recorded #1. Remove weight, then press Enter.");
            calState = CAL_WAIT_REMOVE1;
            break;

          case CAL_WAIT_REMOVE1:
            Serial.println("Place known weight (#2) then press Enter.");
            calState = CAL_WAIT_PLACE2;
            break;

          case CAL_WAIT_PLACE2:
            calSamples[1] = scale.read_average(10) - scale.get_offset();
            Serial.println("Recorded #2. Remove weight, then press Enter.");
            calState = CAL_WAIT_REMOVE2;
            break;

          case CAL_WAIT_REMOVE2:
            Serial.println("Place known weight (#3) then press Enter.");
            calState = CAL_WAIT_PLACE3;
            break;

          case CAL_WAIT_PLACE3:
            calSamples[2] = scale.read_average(10) - scale.get_offset();
            Serial.println("Enter known weight in grams (number only), then press Enter:");
            calState = CAL_INPUT_WEIGHT;
            break;

          default:
            break;
        }
      }
    } else {
      if (calState == IDLE && (c == 'T' || c == 't')) {
        scale.tare();
        Serial.println("Tared.");
      } else if (calState == IDLE && (c == 'C' || c == 'c')) {
        Serial.println("Calibration: remove all weight, then press Enter.");
        calState = CAL_WAIT_CLEAR;
      } else {
        inputLine += c;
      }
    }
  }

  // Display readings
  if (scaleFactor > 0.0f) {
    float grams = scale.get_units(5);
    Serial.print("g: ");
    Serial.println(grams, 2);
  } else {
    long reading = scale.read_average(5);
    Serial.print("raw: ");
    Serial.println(reading);
  }

  delay(150);
}
