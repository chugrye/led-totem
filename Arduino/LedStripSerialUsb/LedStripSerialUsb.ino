#include <FastLED.h>
#include <Arduino.h>

//#define PIN1            6
//#define PIN2            5
#define LED_PIN 6

#define NUMPIXELS     30 
#define NUMLINES    1
CRGB leds[NUMLINES][NUMPIXELS];
int framesPerSecond = 1;
int delayval = 1000/framesPerSecond; 

bool needsData = true;
byte pixelBuffer[3];

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds[0], NUMPIXELS);

  //Serial.begin(115200);
}

void printPixelColor(byte red, byte green, byte blue, int ledIndex) {
  Serial.print("LED#");
  Serial.print(ledIndex);
  Serial.print(" RGB: ");
  Serial.print(red);
  Serial.print(",");
  Serial.print(green);
  Serial.print(",");
  Serial.println(blue);
}

void loop(){
  if(Serial.available())
  {
    for (int line = 0; line < NUMLINES; line++)
    {
      for (int pixel = 0; pixel < 30; pixel++)
      {
        Serial.readBytes(pixelBuffer, 3);
        if (pixel == 0) {
          leds[line][pixel].r = 0;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 1) {
          leds[line][pixel].r = 0x01;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 2) {
          leds[line][pixel].r = 0x02;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 3) {
          leds[line][pixel].r = 0x03;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 4) {
          leds[line][pixel].r = 0x04;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 5) {
          leds[line][pixel].r = 0x10;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 6) {
          leds[line][pixel].r = 0x20;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 7) {
          leds[line][pixel].r = 0x30;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } else if (pixel == 8) {
          leds[line][pixel].r = 0x40;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = 0;
        } if (pixel > 9 && pixel < 19) {
          leds[line][pixel].r = 0;
          leds[line][pixel].g = pixel;
          leds[line][pixel].b = 0;
        } else if (pixel > 19 && pixel < 29) {
          leds[line][pixel].r = 0;
          leds[line][pixel].g = 0;
          leds[line][pixel].b = pixel;
//        } else {
//          leds[line][pixel].r = pixelBuffer[0];
//          leds[line][pixel].g = pixelBuffer[1];
//          leds[line][pixel].b = pixelBuffer[2];
        }
        printPixelColor(pixelBuffer[0], pixelBuffer[1], pixelBuffer[2], pixel);
        //FastLED.show();
      }
    }
    while(Serial.available()) {
      // read remaining stuff
      Serial.read();
    }
    needsData = false;
  } else if (!needsData) {
    FastLED.show();
    needsData = true;
  }
}
