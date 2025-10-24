//
//    FILE: HX_plotter.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: HX711 demo
//     URL: https://github.com/RobTillaart/HX711


#include "HX711.h"



//OFFSET: -4627811
//SCALE:  -1026.388061



HX711 scale;

//  adjust pins if needed
uint8_t dataPin = 2;
uint8_t clockPin = 3;

float f;


void setup()
{
  Serial.begin(115200);
  //  Serial.println();
  //  Serial.println(__FILE__);
  //  Serial.print("HX711_LIB_VERSION: ");
  //  Serial.println(HX711_LIB_VERSION);
  //  Serial.println();

  scale.begin(dataPin, clockPin);

  //  TODO find a nice solution for this calibration..
  //  load cell factor 20 KG
  //  scale.set_scale(127.15);
  //  load cell factor 5 KG
  scale.set_gain(128);
  scale.set_scale(-1026.388061);       // TODO you need to calibrate this yourself.
  scale.set_offset(-4627811);
  //  reset the scale to zero = 0
  scale.tare();
}


void loop()
{
    // Check for serial input
  if (Serial.available() > 0)
  {
    char c = Serial.read();
    if (c == 't' || c == 'T')
    {
      scale.tare();
      Serial.println("Tare complete");
    }
  }

  //  continuous scale 4x per second
  f = scale.get_units(1);
  Serial.println(f);
  delay(10);
}


//  -- END OF FILE --

