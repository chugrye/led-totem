#define redPin 6
#define greenPin 5
#define bluePin 3

#define NUMPIXELS     30

const long interval = 300;
unsigned long previousMillis = 0;

bool needsData = true;

int ledIndex = 0;
byte pixelBuffer[3];
byte curFrame[30][3];

void setup(){
  //start the Serial connection
  //Serial.begin(115200);
  Serial.begin(9600);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void setColor(byte red, byte green, byte blue, int ledIndex)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);

  Serial.print("LED: ");
  Serial.print(ledIndex);
  Serial.print(" RGB: ");
  Serial.print(red);
  Serial.print(",");
  Serial.print(green);
  Serial.print(",");
  Serial.println(blue);
}

void loop(){
  unsigned long currentMillis = millis();
  if(Serial.available())
  {
    for (int i = 0; i < NUMPIXELS; i++) {
      Serial.readBytes(pixelBuffer, 3);
      curFrame[i][0] = pixelBuffer[0];
      curFrame[i][1] = pixelBuffer[1];
      curFrame[i][2] = pixelBuffer[2];
    }
    while(Serial.available()) {
      // read remaining stuff
      Serial.read();
    }
    ledIndex = 0;
    needsData = false;
  } else if (!needsData) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (ledIndex < NUMPIXELS) {
        byte* curLed = curFrame[ledIndex];
        setColor(curLed[0],curLed[1],curLed[2], ledIndex);
        ledIndex++;
      } else {
        needsData = true;
      }
    }
  }
}
