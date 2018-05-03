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
#define NUMLINES      7
CRGB leds[NUMLINES][NUMPIXELS];

void setup() {
	FastLED.addLeds<NEOPIXEL, LEDPIN1>(leds[0], NUMPIXELS);
	FastLED.addLeds<NEOPIXEL, LEDPIN2>(leds[1], NUMPIXELS);
	FastLED.addLeds<NEOPIXEL, LEDPIN3>(leds[2], NUMPIXELS);
	FastLED.addLeds<NEOPIXEL, LEDPIN4>(leds[3], NUMPIXELS);
	FastLED.addLeds<NEOPIXEL, LEDPIN5>(leds[4], NUMPIXELS);
	FastLED.addLeds<NEOPIXEL, LEDPIN6>(leds[5], NUMPIXELS);
	FastLED.addLeds<NEOPIXEL, LEDPIN7>(leds[6], NUMPIXELS);

	// Open serial port
	Serial.begin(115200);

	// Set initial brightness to 20%
	FastLED.setBrightness(51);
}

// Error variables
int errorCount = 0;
bool animReadError = false;

// Setting variables
uint8_t brightnessVal = 20;

// Global variables
int command;

void loop() {
	while (Serial.available() == 0);
	command = Serial.read();

	//command = 0xFE;
	switch (command)
	{
		// Show frame
		case 0x00:
			showFrameHandler();
			break;

		// Load frame
		case 0x01:
			loadFrameHandler();
			break;

		// Settings
		case 0x40:
			settingsHandler();
			break;

		// Rainbow Cycle
		case 0xFE:
			rainbowCycleAnimation();
			break;

		// Start new animation
		case 0x03:
			newAnimationHandler();
			break;
	}
}

void showFrameHandler() {
	// Update LEDs
	FastLED.show();

	// Tell the controller we're ready
	// We don't want to be receiving serial data during leds.show() because data will be dropped
	Serial.write(0x00);
}

void loadFrameHandler() {
	byte byteReadCount = 0;
	byte pixelBuffer[3];

	while (Serial.available() == 0);
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
}

void settingsHandler() {
	int settingCommand;
	// Find out which setting is being accessed
	while (Serial.available() == 0);
	settingCommand = Serial.read();

	switch (settingCommand) {
		case 0x05:
			brightnessSetting();
			break;
	}

	// Broadcast Settings Finished
	Serial.write(0x40);
}

void brightnessSetting() {
	int readBrightness;
	// Find out which setting is being accessed
	while (Serial.available() == 0);
	readBrightness = Serial.read();

	if (readBrightness != -1) {
		brightnessVal = getBrightnessFromScale(readBrightness);
		FastLED.setBrightness(brightnessVal);
	}

	FastLED.show();
}

uint8_t getBrightnessFromScale(byte scaleVal) {
	return scaleVal * (255 / 100);
}

void newAnimationHandler() {
	animReadError = false;
	FastLED.clear();
	FastLED.show();
	Serial.write(0x03);
}

void rainbowCycleAnimation() {
	byte rainbowTrailFrameCount = 0;
	byte red, green, blue;
	while (Serial.available() == 0) {
		for (byte line = 0; line < NUMLINES; line++) {
			for (byte pixel = 0; pixel < NUMPIXELS; pixel++) {
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
