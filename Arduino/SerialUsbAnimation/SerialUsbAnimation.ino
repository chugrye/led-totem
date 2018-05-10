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

#define ANIM_COMMAND 0xC0

// Setting variables
uint8_t brightnessVal = 20;

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
	FastLED.setBrightness(getBrightnessFromScale(brightnessVal));
}

// Error variables
int errorCount = 0;
bool animReadError = false;

// Global variables
int command;

int lastCommand;
int lastAnimationCommand;

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

		// Stored Animation
		case ANIM_COMMAND:
			storedAnimationHandler(false);
			break;

		// Start new animation
		case 0x03:
			newAnimationHandler();
			break;
	}
	lastCommand = command;
}

// Greatest common denominator
int gcd(int a, int b)
{
	// base case
	if (a == b)
		return a;

	// a is greater
	if (a > b)
		return gcd(a - b, b);
	return gcd(a, b - a);
}

// Function to return LCM of two numbers
int lcm(int a, int b)
{
	return (a*b) / gcd(a, b);
}

uint32_t colorWheel(uint16_t pos) {
	pos = 765 - pos;
	if (pos < 255) {
		return ((uint32_t)(255 - pos) << 16) | ((uint32_t)(0) << 8) | (pos);
	}
	else if (pos < 510) {
		pos -= 255;
		return ((uint32_t)(0) << 16) | ((uint32_t)(pos) << 8) | (255 - pos);
	}
	else {
		pos -= 510;
		return ((uint32_t)(pos) << 16) | ((uint32_t)(255 - pos) << 8) | (0);
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

	// Need to reinitialize the stored animation if interupted
	if (lastCommand == ANIM_COMMAND) {
		command = ANIM_COMMAND;
		storedAnimationHandler(true);
	}
	else {
		// Broadcast Settings Finished
		Serial.write(0x40);
	}
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

void setPixel(int line, int Pixel, CRGB color) {
	leds[line][Pixel] = color;
}

void setPixel(int line, int Pixel, uint32_t color) {
	leds[line][Pixel] = color;
}

void setPixel(int line, int Pixel, byte red, byte green, byte blue) {
	leds[line][Pixel].r = green;
	leds[line][Pixel].g = red;
	leds[line][Pixel].b = blue;
}

void setAll(int line, byte red, byte green, byte blue) {
	for (int i = 0; i < NUMPIXELS; i++) {
		setPixel(line, i, red, green, blue);
	}
}

void storedAnimationHandler(bool useLastAnimation) {
	int anim;
	// Find out which setting is being accessed
	if (useLastAnimation) {
		anim = lastAnimationCommand;
	} else {
		while (Serial.available() == 0);
		anim = Serial.read();
	}

	FastLED.clear();

	switch (anim) {
		case 0x02:
			rainbowCycleAnimation();
			break;
		case 0x03:
			meteorRainAnimation();
			break;
		case 0x04:
			fireworkAnimation();
			break;
	}
	lastAnimationCommand = anim;
}

void rainbowCycleAnimation() {
	int rainbowTrailFrameCount = 0;
	byte numIntervals = 45;

	int maxCycleIndex = lcm(numIntervals, NUMPIXELS);
	byte red, green, blue;
	byte counter = 0;
	while (Serial.available() == 0) {
		for (byte line = 0; line < NUMLINES; line++) {
			for (byte pixel = 0; pixel < NUMPIXELS; pixel++) {
				byte factor = (rainbowTrailFrameCount + pixel) % numIntervals;
				uint16_t colorIndex = factor * (765 / numIntervals);
				leds[line][pixel] = colorWheel(colorIndex);
			}
		}
		FastLED.show();
		rainbowTrailFrameCount++;
		rainbowTrailFrameCount = rainbowTrailFrameCount % maxCycleIndex;
		delay(50);
	}
}

int distanceFromCenter(int line, int pixel, int i, int j) {
	return abs(i - line) + abs(j - pixel);
}

void fireworkAnimation() {
	while (Serial.available() == 0) {
		uint16_t colorIndex = random(0, 765);
		int fireworkRadius = random(4, 6);
		int startingLine = random(2, NUMLINES - 3);
		int startingPixel = random(fireworkRadius, NUMPIXELS - fireworkRadius - 1);
		int pixel;
		int line;
		uint16_t colorSpread = random(70, 120);
		for (int i = 0; i < (fireworkRadius*2); i++) {
			line = (startingLine - fireworkRadius + i);
			for (int j = 0; j < (fireworkRadius*2); j++) {
				pixel = (startingPixel - fireworkRadius + j);
				if (line >= 0 && line < NUMLINES && pixel >= 0 && pixel < NUMPIXELS) {
					int colorFactor = distanceFromCenter(startingLine, startingPixel, line, pixel);
					if (colorFactor <= fireworkRadius) {
						int colorFade = colorFactor * (255 / fireworkRadius);
						CRGB startColor = colorWheel((colorIndex + colorFactor * colorSpread) % 765);
						CRGB nextColor = CRGB(max(startColor.r - colorFade, 0), max(startColor.g - colorFade, 0), max(startColor.b - colorFade, 0));
						setPixel(line, pixel, nextColor);
					}
				}
			}
		}
		FastLED.show();
		delay(50);
		byte delayFactor = random(15, 40);
		for (byte i = 0; i < delayFactor; i++) {
			for (byte j = 0; j < NUMLINES; j++) {
				for (byte k = 0; k < NUMPIXELS; k++) {
					if ((random(10) > 5)) {
						leds[j][k].fadeToBlackBy(64);
					}
				}
			}
			FastLED.show();
			delay(50);
		}
	}
}

void setMeteorPixels(byte line, int i, int j, uint16_t colorIndex) {
	if ((i - j < NUMPIXELS) && (i - j >= 0)) {
		setPixel(line, i - j, colorWheel(colorIndex));
	}
}

void meteorRainAnimation() {
	byte red;
	byte green;
	byte blue;
	byte meteorSize = 7;
	byte meteorTrailDecay = 64;
	byte meteorRandomDecay = true;
	int SpeedDelay = 30;
	// red/green/blue = meteor color
	// meteorSize = #LEDs for the meteor (main part, excluding the trail)
	// meteorTrailDecay = fading speed of the trail. 64 = dim by 25% (64/256ths); lower number = longer tail
	// meteorRandomDecay = true of false. False = smooth tail, true = tail that breaks up in pieces
	// SpeedDelay = pause in ms after drawing a cycle - lower number = faster meteor

	byte line1 = random(0, NUMLINES - 1);
	byte line2 = random(0, NUMLINES - 1);
	byte line3 = random(0, NUMLINES - 1);

	byte c1;
	byte c2;
	byte c3;

	int i = 0;
	int maxI = NUMPIXELS * 3;
	while (Serial.available() == 0) {
		i++;
		c1 = i % maxI;
		c2 = ((i - maxI/4) % maxI + maxI) % maxI;
		c3 = ((i - maxI*2/4) % maxI + maxI) % maxI;

		if (c1 == 0) {
			setAll(line1, 0, 0, 0);
			line1 = random(0, NUMLINES - 1);
		}
		if (c2 == 0) {
			setAll(line2, 0, 0, 0);
			line2 = random(0, NUMLINES - 1);
		}
		if (c3 == 0) {
			setAll(line3, 0, 0, 0);
			line3 = random(0, NUMLINES - 1);
		}
		
		// draw meteor
		for (int j = 0; j < meteorSize; j++) {
			uint16_t colorIndex = (i * 3) % 765;
			setMeteorPixels(line1, c1, j, colorIndex);
			setMeteorPixels(line2, c2, j, colorIndex);
			setMeteorPixels(line3, c3, j, colorIndex);
		}

		// fade brightness all LEDs one step
		for (int j = 0; j < NUMPIXELS; j++) {
			if ((!meteorRandomDecay) || (random(10) > 5)) {
				//leds[line][j].fadeToBlackBy(meteorTrailDecay);
				leds[line1][j].fadeToBlackBy(meteorTrailDecay);
				leds[line2][j].fadeToBlackBy(meteorTrailDecay);
				leds[line3][j].fadeToBlackBy(meteorTrailDecay);
			}
		}

		FastLED.show();
		delay(SpeedDelay);
	}
}

void fireAnimation(int cooling, int sparkling)
{
	// Array of temperature readings at each simulation cell
	static byte heat[NUMPIXELS];

	for (int line = 0; line < NUMLINES; line++) {
		// Step 1.  Cool down every cell a little
		int cooldown;
		for (int i = 0; i < NUMPIXELS; i++) {
			//heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / NUMPIXELS) + 2));
			cooldown = random(0, ((cooling * 10) / NUMPIXELS) + 2);

			if (cooldown > heat[i]) {
				heat[i] = 0;
			}
			else {
				heat[i] = heat[i] - cooldown;
			}
		}

		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		for (int k = NUMPIXELS - 1; k >= 2; k--) {
			heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
		}

		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		if (random8() < sparkling) {
			//int y = random8(7);
			//heat[y] = qadd8(heat[y], random8(160, 255));
			int y = random(7);
			heat[y] = heat[y] + random(160, 255);
		}

		// Step 4.  Map from heat cells to LED colors
		for (int j = 0; j < NUMPIXELS; j++) {
			CRGB color = HeatColor(heat[j]);
			int pixelnumber;
			pixelnumber = (NUMPIXELS - 1) - j;
			//if (gReverseDirection) {
			//	pixelnumber = (NUM_LEDS - 1) - j;
			//}
			//else {
			//	pixelnumber = j;
			//}
			leds[line][pixelnumber] = color;
		}
	}
}
