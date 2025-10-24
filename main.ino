#include "HX711.h"

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;

enum CalState {
  IDLE,
  CAL_WAIT_CLEAR,
  CAL_WAIT_PLACE
};

CalState calState = IDLE;
const int NUM_CAL = 4;
float calTargets[NUM_CAL] = {50.0f, 100.0f, 150.0f, 250.0f};
long calSamples[NUM_CAL] = {0, 0, 0, 0};
int calIndex = 0;
String inputLine = "";
float scaleFactor = 0.0;

void setup() {
  Serial.begin(57600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("Starting calibration...");
  Serial.println("Ensure the scale is empty, then press Enter.");
  Serial.println("You will be prompted for 50g, 100g, 150g, and 250g.");
  calState = CAL_WAIT_CLEAR;
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

      switch (calState) {
        case CAL_WAIT_CLEAR: {
          scale.tare();
          calIndex = 0;
          Serial.println("Zero set.");
          Serial.print("Place "); Serial.print(calTargets[calIndex], 0); Serial.println("g and press Enter.");
          calState = CAL_WAIT_PLACE;
          break;
        }

        case CAL_WAIT_PLACE: {
          long raw = scale.read_average(10) - scale.get_offset();
          calSamples[calIndex] = raw;
          Serial.print("Recorded for "); Serial.print(calTargets[calIndex], 0); Serial.println("g.");
          calIndex++;
          if (calIndex < NUM_CAL) {
            Serial.print("Replace with "); Serial.print(calTargets[calIndex], 0); Serial.println("g and press Enter.");
          } else {
            // Compute scale factor as average of raw/grams across all points
            double sumRatio = 0.0;
            for (int i = 0; i < NUM_CAL; ++i) {
              sumRatio += (double)calSamples[i] / (double)calTargets[i];
            }
            scaleFactor = (float)(sumRatio / (double)NUM_CAL);
            scale.set_scale(scaleFactor);
            Serial.print("Calibration complete. scaleFactor=");
            Serial.println(scaleFactor, 6);
            float gramsNow = scale.get_units(5);
            Serial.print("Current reading (g): ");
            Serial.println(gramsNow, 2);
            Serial.println("Commands: T=tare, C=calibrate again");
            calState = IDLE;
          }
          break;
        }

        default:
          break;
      }
    } else {
      if (calState == IDLE && (c == 'T' || c == 't')) {
        scale.tare();
        Serial.println("Tared.");
      } else if (calState == IDLE && (c == 'C' || c == 'c')) {
        Serial.println("Calibration: ensure scale is empty, then press Enter.");
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
