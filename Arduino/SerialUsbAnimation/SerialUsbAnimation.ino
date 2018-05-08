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
	}
	lastAnimationCommand = anim;
}

void setMeteorPixels(byte line, int i, int j, byte red, byte green, byte blue) {
	if ((i - j < NUMPIXELS) && (i - j >= 0)) {
		//setPixel(line, i - j, red, green, blue);
		setPixel(line, i - j, red, green, blue);
	}
}

byte getRandomByte(byte seed, byte max) {
	return seed * (seed + 2) % max;
}

void meteorRainAnimation() {
	byte red = 0xff;
	byte green = 0xff;
	byte blue = 0xff;
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

		byte randomSeed = line1 + line2 + line3;
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
			setMeteorPixels(line1, c1, j, red, green, blue);
			//if ((i - j < NUMPIXELS) && (i - j >= 0)) {
			//	//setPixel(line, i - j, red, green, blue);
			//	setPixel(line1, i - j, red, green, blue);
			//}
			setMeteorPixels(line2, c2, j, red, green, blue);
			setMeteorPixels(line3, c3, j, red, green, blue);
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
