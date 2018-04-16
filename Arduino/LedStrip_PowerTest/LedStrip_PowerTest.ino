#include <FastLED.h>

#define LEDPIN1            2

#define NUMPIXELS     30 
#define NUMLINES    1
CRGB leds[NUMLINES][NUMPIXELS];

void setup() {
  FastLED.addLeds<NEOPIXEL, LEDPIN1>(leds[0], NUMPIXELS);
  Serial.begin(115200);
}

byte count = 0;
void loop() {
//    Serial.print("COUNT: ");
//    Serial.println(count);
  byte red, green, blue;
  for (byte pixel = 0; pixel < NUMPIXELS; pixel++)
  {
    byte factor = count + pixel;
    byte index = (factor / 15) % 3;
    byte val = (factor % 15) * 17;
    if (index == 0) {
       red = val;
       green = 255 - val;
       blue = 0;
    } else if (index == 1) {
      red = 255 - val;
      green = 0;
      blue = val;
    } else if (index == 2) {
      red = 0;
      green = val;
      blue = 255 - val;
    }
    leds[0][pixel].r = red;
    leds[0][pixel].g = green;
    leds[0][pixel].b = blue;

//      Serial.print("RGB: ");
//      Serial.print(red);
//      Serial.print(", ");
//      Serial.print(green);
//      Serial.print(", ");
//      Serial.print(blue);
//      Serial.print("\n");
//      leds[0][pixel].r = 255;
//      leds[0][pixel].g = 255;
//      leds[0][pixel].b = 255;
  }
  FastLED.show();
  count++;
  count = count % 45;
  delay(50);
}



