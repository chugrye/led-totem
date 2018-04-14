#include <FastLED.h>
#include <Arduino.h>

#define LEDPIN1            2

#define NUMPIXELS     30 
#define NUMLINES    1
CRGB leds[NUMLINES][NUMPIXELS];

bool firstTime = true;

void setup() {
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LEDPIN1>(leds[0], NUMPIXELS);

  // Open serial port and tell the controller we're ready.
  //Serial.begin(115200);
}

byte count = 0;
void loop() {
//    Serial.print("COUNT: ");
//    Serial.println(count);
  int red, green, blue;
  for (int pixel = 0; pixel < NUMPIXELS; pixel++)
  {
    int factor = (count + pixel);
    int index = (factor / 15) % 3;
    int subVal = ((factor / 15) * 255);
    int val = (count + pixel) * 17 - subVal;
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



