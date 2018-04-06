#include <FastLED.h>
#include <Arduino.h>

#define PIN1            6
#define PIN2            5

#define NUMPIXELS     30 
#define NUMLINES	  2
CRGB leds[NUMLINES][NUMPIXELS];
int framesPerSecond = 1;
int delayval = 1000/framesPerSecond; 

void setup() {
#if defined (__AVR_ATtiny85__)
	if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
	Serial.begin(9600);
	FastLED.addLeds<NEOPIXEL, PIN1>(leds[0], NUMPIXELS);
	FastLED.addLeds<NEOPIXEL, PIN2>(leds[1], NUMPIXELS);

	//FastLED.show();

	// Open serial port and tell the controller we're ready.
	Serial.begin(1228800);
	Serial.write(0x00);
}

//bool brightnessDecreasing = true;
//int designPatternOffset = 0;
byte pixelBuffer[3];
bool write = true;
void loop() {
	// Read a command
	while (Serial.available() == 0);
	byte command = Serial.read();
	byte count;
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
		while (Serial.available() == 0);
//		count = Serial.read();
		for (int line = 0; line < NUMLINES; line++)
		{
			for (int pixel = 0; pixel < 30; pixel++)
			{
				Serial.readBytes(pixelBuffer, 3);
				leds[line][pixel].r = pixelBuffer[0];
				leds[line][pixel].g = pixelBuffer[1];
				leds[line][pixel].b = pixelBuffer[2];
			}
		}
		//FastLED.show();
		break;
		// Clear
	case 0xFF:
		FastLED.clear();
		FastLED.show();
		break;
	}
}



