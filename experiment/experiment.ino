#include "HX711.h"

const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN  = 3;
HX711 scale;

// Calibration constants
const float m = -0.0013;
const float b = -4237.73 + 1527 + 247.4;

// Filter parameters
#define EMA_ALPHA 0.3        // 0.3 = smoother, 0.6 = faster
#define DRIFT_ALPHA 0.001    // baseline drift tracking
#define THRESHOLD 6.0        // grams change to detect candy event
#define HYSTERESIS 3.0
#define WINDOW_SIZE 3

float ema = 0, baseline = 0, prevFiltered = 0;
float readings[WINDOW_SIZE];
int idx = 0;
bool initEMA = false;
bool inStep = false;
float lastStable = 0;

// Median of 3 helper
float median3(float a, float b, float c) {
  if ((a >= b && a <= c) || (a <= b && a >= c)) return a;
  if ((b >= a && b <= c) || (b <= a && b >= c)) return b;
  return c;
}

float EMA(float x, float alpha) {
  if (!initEMA) { ema = x; initEMA = true; }
  ema += alpha * (x - ema);
  return ema;
}

void setup() {
  Serial.begin(230400);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_gain(128);
  scale.tare();
  for (int i = 0; i < WINDOW_SIZE; i++) readings[i] = 0;
}

void loop() {
  if (!scale.is_ready()) {
    Serial.println("HX711 not found");
    return;
  }

  long raw = scale.read();
  float grams = m * raw + b;

  // median-of-3 smoothing
  readings[idx] = grams;
  idx = (idx + 1) % WINDOW_SIZE;
  float g_med = median3(readings[0], readings[1], readings[2]);

  // exponential smoothing
  float g_ema = EMA(g_med, EMA_ALPHA);

  // drift compensation
  baseline += DRIFT_ALPHA * (g_ema - baseline);
  float g_corr = g_ema - baseline;

  // step detection
  float diff = g_corr - lastStable;
  if (!inStep && fabs(diff) > THRESHOLD) {
    inStep = true;
  }
  if (inStep && fabs(diff) < HYSTERESIS) {
    inStep = false;
    float delta = g_corr - lastStable;
    lastStable = g_corr;
    if (fabs(delta) > 2) {
      if (delta < 0) Serial.println("Candy removed");
      else Serial.println("Candy added");
    }
  }

  Serial.print("Raw: "); Serial.print(raw);
  Serial.print("\tFilt: "); Serial.print(g_corr, 2);
  Serial.print("\tBase: "); Serial.println(baseline, 2);
}
