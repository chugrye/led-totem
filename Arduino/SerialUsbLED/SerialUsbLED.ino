#define redPin 6
#define greenPin 5
#define bluePin 3

const long interval = 1000;
unsigned long previousMillis = 0;
char ledIndex = 0;

char curFrame[30][3];

bool needsData = true;

void setup(){
  //start the Serial connection
  //Serial.begin(115200);
  Serial.begin(9600);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void setColor(int red, int green, int blue)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void loop(){
  unsigned long currentMillis = millis();
  if (Serial.available()){
    char ledVals[5];
    for (char i = 0; i < sizeof(ledVals); i++) {
      ledVals[i] = Serial.read();
    }
    char ledNum = ledVals[1];
    curFrame[ledNum][0] = ledVals[2];
    curFrame[ledNum][1] = ledVals[2];
    curFrame[ledNum][2] = ledVals[2];
    ledIndex = 0;
    needsData = false;
  } else if (!needsData) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      if (ledIndex < sizeof(curFrame)) {
        char (*curLed)[3];
        curLed = &curFrame[ledIndex]; 
        //char * curLed[3] = curFrame[ledIndex];
        int redVal = (*curLed[0] - '0');
        int greenVal = (*curLed[1] - '0');
        int blueVal = (*curLed[2] - '0');
        setColor(redVal,greenVal,blueVal);
        ledIndex++;
      } else {
        needsData = true;
      }
    }
  }
}
