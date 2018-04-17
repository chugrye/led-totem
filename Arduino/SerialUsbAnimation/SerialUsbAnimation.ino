#include <FastLED.h>
#include <Arduino.h>

#define LEDPIN1            2
#define LEDPIN2            3
#define LEDPIN3            4
#define LEDPIN4            5
#define LEDPIN5            6
#define LEDPIN6            7
#define LEDPIN7            8

#define NUMPIXELS     30 
#define NUMLINES	  7
CRGB leds[NUMLINES][NUMPIXELS];

void setup() {
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
FastLED.addLeds<NEOPIXEL, LEDPIN1>(leds[0], NUMPIXELS);
FastLED.addLeds<NEOPIXEL, LEDPIN2>(leds[1], NUMPIXELS);
FastLED.addLeds<NEOPIXEL, LEDPIN3>(leds[2], NUMPIXELS);
FastLED.addLeds<NEOPIXEL, LEDPIN4>(leds[3], NUMPIXELS);
FastLED.addLeds<NEOPIXEL, LEDPIN5>(leds[4], NUMPIXELS);
FastLED.addLeds<NEOPIXEL, LEDPIN6>(leds[5], NUMPIXELS);
FastLED.addLeds<NEOPIXEL, LEDPIN7>(leds[6], NUMPIXELS);

	// Open serial port and tell the controller we're ready.
	Serial.begin(115200);
}

bool showFrame = false;

int errorCount = 0;
byte byteReadCount = 0;
bool animReadError = false;

byte pixelBuffer[3];
void loop() {
	// Read a command
	while (Serial.available() == 0);
	byte command = Serial.read();

	switch (command)
	{
		// Show frame
	case 0x00:
		// Update LEDs
		FastLED.show();

		// Tell the controller we're ready
		// We don't want to be receiving serial data during leds.show() because data will be dropped
		Serial.write(0x00);
		break;

		// Load frame
	case 0x01:
		// Read and update pixels
		for (int line = 0; line < NUMLINES; line++)
		{
			for (int pixel = 0; pixel < NUMPIXELS; pixel++)
			{
				byteReadCount = Serial.readBytes(pixelBuffer, 3);
				if (byteReadCount < 3) {
					errorCount++;
					leds[line][pixel].r = 0;
					leds[line][pixel].g = 0;
					leds[line][pixel].b = 0;
					animReadError = true;
					break;
				}
				leds[line][pixel].r = pixelBuffer[0];
				leds[line][pixel].g = pixelBuffer[1];
				leds[line][pixel].b = pixelBuffer[2];
			}
		}
		if (animReadError) {
			while (Serial.available() > 0) {
				Serial.read();
			}
			Serial.write(0xAA);
			Serial.print("E:");
			Serial.print(errorCount);
		}
		break;

		// Rainbow Cycle
	case 0xFE:
		rainbowCycleAnimation();
		break;

		// Clear
	case 0x03:
		animReadError = false;
		FastLED.clear();
		FastLED.show();
		Serial.write(0x03);
		break;
	}
}

void rainbowCycleAnimation() {
	byte rainbowTrailFrameCount = 0;
	byte red, green, blue;
	while (Serial.available() == 0) {
    for (byte line = 0; line < NUMLINES; line++) {
  		for (byte pixel = 0; pixel < NUMPIXELS; pixel++)
  		{
  			byte factor = rainbowTrailFrameCount + pixel;
  			byte index = (factor / 15) % 3;
  			byte val = (factor % 15) * 17;
  			if (index == 0) {
  				red = val;
  				green = 255 - val;
  				blue = 0;
  			}
  			else if (index == 1) {
  				red = 255 - val;
  				green = 0;
  				blue = val;
  			}
  			else if (index == 2) {
  				red = 0;
  				green = val;
  				blue = 255 - val;
  			}
  			leds[line][pixel].r = red;
  			leds[line][pixel].g = green;
  			leds[line][pixel].b = blue;
  		}
    }
		FastLED.show();
		rainbowTrailFrameCount++;
		rainbowTrailFrameCount = rainbowTrailFrameCount % 45;
		delay(50);
	}
}
