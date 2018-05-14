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
byte staticMessageDelay = 0x00;
bool isStaticWordAnimation = false;

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

		// Start new static message
		case 0x31:

			newStaticMessageHandler();
			break;
	}
	lastCommand = command;
}

int getStaticMessageDelay() 
{
	if(staticMessageDelay > 0x08)
	{
		staticMessageDelay = 0x01;
	}
	//return 2000 / 8 * (int)staticMessageDelay;
	return 1000;
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

void showFrameHandler() {
	// Update LEDs
	FastLED.show();

	// Tell the controller we're ready
	// We don't want to be receiving serial data during leds.show() because data will be dropped
	if (isStaticWordAnimation == true) {
		delay(getStaticMessageDelay());
	}
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
	return scaleVal * (255*1.0 / 100);
}

void newAnimationHandler() {
	isStaticWordAnimation = false;
	animReadError = false;
	FastLED.clear();
	FastLED.show();
	Serial.write(0x03);
}

void newStaticMessageHandler() {
	while (Serial.available() == 0);
	staticMessageDelay = Serial.read();
	isStaticWordAnimation = true;
	animReadError = false;
	FastLED.clear();
	FastLED.show();
	Serial.write(0x31);
}

// Color Utilities
// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color)
{
	return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
{
	return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
{
	return color & 0xFF;
}

uint32_t Color(uint8_t red, uint8_t green, uint8_t blue) {
	return ((uint32_t)(red) << 16) | ((uint32_t)(green) << 8) | (blue);
}

// Return color, dimmed by 75% (used by scanner)
uint32_t DimColor(uint32_t color, uint8_t fadeBy)
{
	uint32_t dimColor = Color(max(Red(color) - fadeBy, 0), max(Green(color) - fadeBy, 0), max(Blue(color) - fadeBy, 0));
	return dimColor;
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
		case 0x05:
			tealPurpleOrangeHelix();
		case 0x30:
			coUsaFlags();
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

void matrixFadeToBlack(bool randomDecay, uint8_t decayRate, byte numFadeLoops, unsigned long delayLength) {
	for (byte i = 0; i < numFadeLoops; i++) {
		for (byte j = 0; j < NUMLINES; j++) {
			for (byte k = 0; k < NUMPIXELS; k++) {
				if (!randomDecay || (random(10) > 5)) {
					leds[j][k].fadeToBlackBy(decayRate);
				}
			}
		}
		FastLED.show();
		delay(delayLength);
	}
}

byte fireFirework(byte pixelTop) {
	byte fireworkRadius = random(3, 5);
	byte minPixelIndex = fireworkRadius + pixelTop;
	// Get closer to top for first firework
	if (pixelTop == 0) {
		minPixelIndex -= 2;
	}
	// Using min( ) to make sure firework doesn't draw way too low
	byte maxPixelIndex = min(NUMPIXELS + 2 - fireworkRadius, minPixelIndex + 12);
	// Can't get to lowest possible index
	if (minPixelIndex > maxPixelIndex) {
		// Return max pixel value
		return NUMPIXELS;
	}

	uint16_t colorIndex = random(0, 765);
	byte colorSpread = random(70, 120);
	byte pixel;
	byte line;
	byte startingLine = random(1, NUMLINES - 1);
	byte startingPixel = random(minPixelIndex, maxPixelIndex);
	byte fireworkDiameter = fireworkRadius * 2 + 1; // 2r + center

	for (byte i = 0; i < fireworkDiameter; i++) {
		line = (startingLine - fireworkRadius + i);
		for (byte j = 0; j < fireworkDiameter; j++) {
			pixel = (startingPixel - fireworkRadius + j);
			// Only draw when within matrix
			if (line >= 0 && line < NUMLINES && pixel >= 0 && pixel < NUMPIXELS) {
				byte distanceFactor = distanceFromCenter(startingLine, startingPixel, line, pixel);
				if (distanceFactor <= fireworkRadius) {
					byte colorFade = distanceFactor * (255.0 / (fireworkRadius + 1));
					CRGB startColor = colorWheel((colorIndex + distanceFactor * colorSpread) % 765);
					CRGB nextColor = CRGB(max(startColor.r - colorFade, 0), max(startColor.g - colorFade, 0), max(startColor.b - colorFade, 0));
					setPixel(line, pixel, nextColor);
				}
			}
		}
	}
	FastLED.show();

	// Returning last firework pixelIndex used
	return startingPixel + fireworkRadius;
}

void fireworkAnimation() {
	while (Serial.available() == 0) {
		byte fireworkCount = random(1, 4);
		byte lastPixelIndex = 0;
		for (int i = 0; i < fireworkCount; i++) {
			lastPixelIndex = fireFirework(lastPixelIndex) + 2;
			delay(random(0, 300));
		}
		byte numFadeLoops = random(18, 40);
		matrixFadeToBlack(true, 64, numFadeLoops, 50);
	}
}

void setMeteorPixels(byte line, int i, int j, uint16_t colorIndex) {
	if ((i - j < NUMPIXELS) && (i - j >= 0)) {
		setPixel(line, i - j, colorWheel(colorIndex));
	}
}

void raindropsAnimation() {
	int lead_dot = 0;
	int angle = 0;
	while (Serial.available() == 0) {

		EVERY_N_MILLIS_I(timerVar, 20)
		//EVERY_N_MILLISECONDS(20)
		{

			uint8_t lead_dot = map(cubicwave8(angle), 0, 256, 0, NUMPIXELS - 1);
			//angle = angle + 1 % 64;

			/*if (lead_dot == 0) {
				stopAnimation = false;
			}

			if (!stopAnimation) {
				leds[1][lead_dot].b = 255;
			}
			if (lead_dot == NUMPIXELS - 1) {
				stopAnimation = true;
			}*/

			leds[1][lead_dot].b = 255;

			fadeToBlackBy(leds[1], NUMPIXELS, 30);
		}

		FastLED.show();
	}

}

struct GhostVars {
	//uint32_t color;
	CRGB color;
	uint8_t horizontalFactor;
	uint8_t verticalFactor;
	uint8_t verticalCount;
	uint8_t nextPixel;
	uint8_t angle;
};

//struct GhostVars setupGhost(uint32_t color) {
struct GhostVars setupGhost(CRGB color) {
	struct GhostVars result;
	//result.color = colorWheel(color);
	result.color = color;

	result.horizontalFactor = 4;
	result.verticalFactor = 2;

	result.verticalCount = 0;
	result.nextPixel = 0;
	result.angle = 0;

	return result;
}

void sendGhost(GhostVars &ghost) {

	ghost.verticalCount = (ghost.verticalCount + 1) % ghost.verticalFactor;
	uint8_t lead_dot = map(cos8(ghost.angle), 0, 256, 0, NUMLINES);

	ghost.angle = ghost.angle + 2 * ghost.horizontalFactor;

	if (ghost.verticalCount == 0) {
		ghost.nextPixel = (ghost.nextPixel + 1) % NUMPIXELS;
	}

	leds[lead_dot][ghost.nextPixel] = ghost.color;
}

void tealPurpleOrangeHelix() {
	CRGB teal = CRGB(17, 159, 159);
	CRGB purple = CRGB(165, 38, 183);
	CRGB orange = CRGB(217, 89, 4);
	triColorGhostHelix(teal, purple, orange, 40);
}

void triColorGhostHelix(CRGB first, CRGB second, CRGB third, byte tickSpeed) {
	struct GhostVars ghost1, ghost2, ghost3, ghost4, ghost5, ghost6;
	ghost1 = setupGhost(first);
	ghost2 = setupGhost(first);
	ghost3 = setupGhost(second);
	ghost4 = setupGhost(second);
	ghost5 = setupGhost(third);
	ghost6 = setupGhost(third);

	ghost2.nextPixel = 9;
	ghost2.angle = 128;

	ghost3.nextPixel = 10;
	ghost4.nextPixel = 19;
	ghost4.angle = 128;

	ghost5.nextPixel = 20;
	ghost6.nextPixel = 29;
	ghost6.angle = 128;

	while (Serial.available() == 0) {
		EVERY_N_MILLISECONDS(tickSpeed)
		{
			sendGhost(ghost1);
			sendGhost(ghost2);
			sendGhost(ghost3);
			sendGhost(ghost4);
			sendGhost(ghost5);
			sendGhost(ghost6);

			for (byte i = 0; i < NUMLINES; i++) {
				fadeToBlackBy(leds[i], NUMPIXELS, 24);
			}
		}

		FastLED.show();
	}
}

// TEST CODE
int bpm = 40;
byte lead_dot = 0;
void snakeTrailAnimation() {
	lead_dot = beatsin8(bpm, 0, NUMPIXELS - 1);
	leds[1][lead_dot].b = 255;

	EVERY_N_MILLISECONDS(5)
	{
		fadeToBlackBy(leds[1], NUMPIXELS, 64);
		FastLED.show();

		int now = millis();

	}
}

void testSparkles() {
	while (Serial.available() == 0) {
		sparkle(0, 5, 5);
		sparkle(0, 15, 5);
		sparkle(0, 25, 5);

		sparkle(2, 5, 5);
		sparkle(2, 15, 5);
		sparkle(2, 25, 5);

		sparkle(4, 5, 5);
		sparkle(4, 15, 5);
		sparkle(4, 25, 5);

		sparkle(6, 5, 5);
		sparkle(6, 15, 5);
		sparkle(6, 25, 5);
		FastLED.show();
		/*delay(150);

		FastLED.clear();
		FastLED.show();
		delay(600);*/
	}
}

void sparkle(byte line, byte pixel, byte length) {
	byte half = (length / 2);
	uint16_t color = colorWheel(510);
	for (byte i = 0; i < length; i++) {
		//byte test = (i * 200.0 / (length - 1));
		byte test = 255;
		byte curPixel = pixel - half + i;
		byte distance = abs(pixel - curPixel);
		byte fadeFactor = distance * (200.0 / half);
		//byte whiteFactor = fadeFactor * (200.0 / half)
		//setPixel(line, curPixel, color);
		//if (distance == 0) {
		//	setPixel(line, curPixel, 255, 255, 255);
		//}
		//else {
		//	setPixel(line, curPixel, 0, test - fadeFactor, 0);
		//}

		setPixel(line, curPixel, 0, sin8(i+22), 0);
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
			cooldown = random(0, ((cooling * 10.0) / NUMPIXELS) + 2);

			if (cooldown > heat[i]) {
				heat[i] = 0;
			}
			else {
				heat[i] = heat[i] - cooldown;
			}
		}

		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		for (int k = NUMPIXELS - 1; k >= 2; k--) {
			heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2])*1.0 / 3;
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

// FLAGS
void coUsaFlags() {
	FastLED.clear();
	setupCoFlag(1);
	setupUsaFlag(16);
	FastLED.show();
}

void setupCoFlag(byte startingIndex) {
	byte st = startingIndex;
	CRGB red = CRGB(255, 0, 0);
	CRGB white = CRGB(255, 255, 255);
	CRGB blue = CRGB(0, 0, 255);
	CRGB yellow = CRGB(255, 255, 0);

	leds[0][st + 0] = blue;
	leds[0][st + 1] = blue;
	leds[0][st + 2] = blue;
	leds[0][st + 3] = blue;
	leds[0][st + 4] = blue;
	leds[0][st + 5] = blue;
	leds[0][st + 6] = blue;
	leds[0][st + 7] = blue;
	leds[0][st + 8] = blue;
	leds[0][st + 9] = blue;
	leds[0][st + 10] = blue;
	leds[0][st + 11] = blue;
	leds[0][st + 12] = blue;

	leds[1][st + 0] = blue;
	leds[1][st + 1] = blue;
	leds[1][st + 2] = blue;
	leds[1][st + 3] = blue;
	leds[1][st + 4] = red;
	leds[1][st + 5] = red;
	leds[1][st + 6] = red;
	leds[1][st + 7] = red;
	leds[1][st + 8] = red;
	leds[1][st + 9] = blue;
	leds[1][st + 10] = blue;
	leds[1][st + 11] = blue;
	leds[1][st + 12] = blue;

	leds[2][st + 0] = white;
	leds[2][st + 1] = white;
	leds[2][st + 2] = white;
	leds[2][st + 3] = white;
	leds[2][st + 4] = red;
	leds[2][st + 5] = yellow;
	leds[2][st + 6] = yellow;
	leds[2][st + 7] = yellow;
	leds[2][st + 8] = white;
	leds[2][st + 9] = white;
	leds[2][st + 10] = white;
	leds[2][st + 11] = white;
	leds[2][st + 12] = white;

	leds[3][st + 0] = white;
	leds[3][st + 1] = white;
	leds[3][st + 2] = white;
	leds[3][st + 3] = white;
	leds[3][st + 4] = red;
	leds[3][st + 5] = yellow;
	leds[3][st + 6] = yellow;
	leds[3][st + 7] = yellow;
	leds[3][st + 8] = white;
	leds[3][st + 9] = white;
	leds[3][st + 10] = white;
	leds[3][st + 11] = white;
	leds[3][st + 12] = white;

	leds[4][st + 0] = white;
	leds[4][st + 1] = white;
	leds[4][st + 2] = white;
	leds[4][st + 3] = white;
	leds[4][st + 4] = red;
	leds[4][st + 5] = yellow;
	leds[4][st + 6] = yellow;
	leds[4][st + 7] = yellow;
	leds[4][st + 8] = white;
	leds[4][st + 9] = white;
	leds[4][st + 10] = white;
	leds[4][st + 11] = white;
	leds[4][st + 12] = white;

	leds[5][st + 0] = blue;
	leds[5][st + 1] = blue;
	leds[5][st + 2] = blue;
	leds[5][st + 3] = blue;
	leds[5][st + 4] = red;
	leds[5][st + 5] = red;
	leds[5][st + 6] = red;
	leds[5][st + 7] = red;
	leds[5][st + 8] = red;
	leds[5][st + 9] = blue;
	leds[5][st + 10] = blue;
	leds[5][st + 11] = blue;
	leds[5][st + 12] = blue;

	leds[6][st + 0] = blue;
	leds[6][st + 1] = blue;
	leds[6][st + 2] = blue;
	leds[6][st + 3] = blue;
	leds[6][st + 4] = blue;
	leds[6][st + 5] = blue;
	leds[6][st + 6] = blue;
	leds[6][st + 7] = blue;
	leds[6][st + 8] = blue;
	leds[6][st + 9] = blue;
	leds[6][st + 10] = blue;
	leds[6][st + 11] = blue;
	leds[6][st + 12] = blue;
}

void setupUsaFlag(byte startingIndex) {
	byte st = startingIndex;
	CRGB red = CRGB(255, 0, 0);
	CRGB white = CRGB(255, 255, 255);
	CRGB blue = CRGB(0, 0, 255);

	leds[0][st + 0] = red;
	leds[0][st + 1] = red;
	leds[0][st + 2] = red;
	leds[0][st + 3] = red;
	leds[0][st + 4] = red;
	leds[0][st + 5] = red;
	leds[0][st + 6] = red;
	leds[0][st + 7] = red;
	leds[0][st + 8] = red;
	leds[0][st + 9] = red;
	leds[0][st + 10] = red;
	leds[0][st + 11] = red;
	leds[0][st + 12] = red;

	leds[1][st + 0] = white;
	leds[1][st + 1] = white;
	leds[1][st + 2] = white;
	leds[1][st + 3] = white;
	leds[1][st + 4] = white;
	leds[1][st + 5] = white;
	leds[1][st + 6] = white;
	leds[1][st + 7] = white;
	leds[1][st + 8] = white;
	leds[1][st + 9] = white;
	leds[1][st + 10] = white;
	leds[1][st + 11] = white;
	leds[1][st + 12] = white;

	leds[2][st + 0] = blue;
	leds[2][st + 1] = blue;
	leds[2][st + 2] = blue;
	leds[2][st + 3] = blue;
	leds[2][st + 4] = blue;
	leds[2][st + 5] = blue;
	leds[2][st + 6] = red;
	leds[2][st + 7] = red;
	leds[2][st + 8] = red;
	leds[2][st + 9] = red;
	leds[2][st + 10] = red;
	leds[2][st + 11] = red;
	leds[2][st + 12] = red;

	leds[3][st + 0] = blue;
	leds[3][st + 1] = blue;
	leds[3][st + 2] = blue;
	leds[3][st + 3] = blue;
	leds[3][st + 4] = blue;
	leds[3][st + 5] = blue;
	leds[3][st + 6] = white;
	leds[3][st + 7] = white;
	leds[3][st + 8] = white;
	leds[3][st + 9] = white;
	leds[3][st + 10] = white;
	leds[3][st + 11] = white;
	leds[3][st + 12] = white;

	leds[4][st + 0] = blue;
	leds[4][st + 1] = blue;
	leds[4][st + 2] = blue;
	leds[4][st + 3] = blue;
	leds[4][st + 4] = blue;
	leds[4][st + 5] = blue;
	leds[4][st + 6] = red;
	leds[4][st + 7] = red;
	leds[4][st + 8] = red;
	leds[4][st + 9] = red;
	leds[4][st + 10] = red;
	leds[4][st + 11] = red;
	leds[4][st + 12] = red;

	leds[5][st + 0] = blue;
	leds[5][st + 1] = blue;
	leds[5][st + 2] = blue;
	leds[5][st + 3] = blue;
	leds[5][st + 4] = blue;
	leds[5][st + 5] = blue;
	leds[5][st + 6] = white;
	leds[5][st + 7] = white;
	leds[5][st + 8] = white;
	leds[5][st + 9] = white;
	leds[5][st + 10] = white;
	leds[5][st + 11] = white;
	leds[5][st + 12] = white;

	leds[6][st + 0] = blue;
	leds[6][st + 1] = blue;
	leds[6][st + 2] = blue;
	leds[6][st + 3] = blue;
	leds[6][st + 4] = blue;
	leds[6][st + 5] = blue;
	leds[6][st + 6] = red;
	leds[6][st + 7] = red;
	leds[6][st + 8] = red;
	leds[6][st + 9] = red;
	leds[6][st + 10] = red;
	leds[6][st + 11] = red;
	leds[6][st + 12] = red;
}
