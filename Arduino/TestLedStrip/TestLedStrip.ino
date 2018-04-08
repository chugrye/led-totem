//#include <Adafruit_NeoPixel.h>
//
////#include <Adafruit_NeoPixel.h>
//#include <Arduino.h>
//
////#define PIN1            6
////#define PIN2            5
////#define LED_PIN 6
//
////#define NUMPIXELS     30 
////#define NUMLINES	  2
////CRGB leds[NUMLINES][NUMPIXELS];
////int framesPerSecond = 1;
////int delayval = 1000/framesPerSecond; 
//
//#define PIN 6
//#define NUM_LEDS 30
//
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
//
//void setup() {
//	strip.begin();
//  strip.show();
//}
//
//void loop() {
//  // set pixel to red, delay(1000)
//  strip.setPixelColor(0, 255, 0, 0);
//  strip.show();
//  delay(1000);
//  // set pixel to off, delay(1000)
//  strip.setPixelColor(0, 0, 0, 0);
//  strip.show();
//  delay(1000);
//}

// Simple NeoPixel test.  Lights just a few pixels at a time so a
// 1m strip can safely be powered from Arduino 5V pin.  Arduino
// may nonetheless hiccup when LEDs are first connected and not
// accept code.  So upload code first, unplug USB, connect pixels
// to GND FIRST, then +5V and digital pin 6, then re-plug USB.
// A working strip will show a few pixels moving down the line,
// cycling between red, green and blue.  If you get no response,
// might be connected to wrong end of strip (the end wires, if
// any, are no indication -- look instead for the data direction
// arrows printed on the strip).
 
#include <Adafruit_NeoPixel.h>
 
#define PIN      6
#define N_LEDS  30
 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);
 
void setup() {
  strip.begin();
}
 
//void loop() {
////  chase(strip.Color(255, 0, 0)); // Red
////  chase(strip.Color(0, 255, 0)); // Green
////  chase(strip.Color(0, 0, 255)); // Blue
//}
// 
//static void chase(uint32_t c) {
//  for(uint16_t i=0; i<strip.numPixels()+4; i++) {
//      strip.setPixelColor(i  , c); // Draw new pixel
//      strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
//      strip.show();
//      delay(25);
//  }
//}

void loop() {
  for(int i=255; i >= 0; i--) {
      byte red = 255 - i;
      fade(strip.Color(red, 0, 0)); // Red
  }
  for(int i=0; i <= 255; i++) {
      byte red = 255 - i;
      fade(strip.Color(red, 0, 0)); // Red
  }
//  chase(strip.Color(0, 255, 0)); // Green
//  chase(strip.Color(0, 0, 255)); // Blue
}
 
static void fade(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i  , c); // Draw new pixel
      //strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
  }
  strip.show();
  //delay();
}
