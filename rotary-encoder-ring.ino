#include <FastLED.h>

/* BEGIN - LED Ring Variables */
#define LED_DATA_PIN 5
#define LED_NUMBER 8
#define LED_BRIGHTNESS 240
#define LED_TYPE WS2812B
#define LED_COLOR_MODEL GRB
CRGB leds[LED_NUMBER];

/* BEGIN – Encoder Variables */
#define ENCODER_CLOCK_PIN 4
#define ENCODER_DATA_PIN 3
#define ENCODER_SWITCH_PIN 7
#define ENCODER_POS_MIN 1
#define ENCODER_POS_MAX 20

const double ENCODER_POS_PER_LED = ((double)ENCODER_POS_MAX / LED_NUMBER);
int iEncoderPos = 1;
int iEncoderPosLast = 1;
byte iHue = HUE_ORANGE;
int iVal;
int iValLast;
boolean bCW;

void doSetShow() {
    int iLastLed = iEncoderPos / ENCODER_POS_PER_LED;
    for (int i = 0; i < LED_NUMBER; i++) {
      byte iV = 0;
      byte iLED = (LED_NUMBER - ENCODER_POS_MIN - i);
      if (i < iLastLed) {
        iV = (i * i + LED_BRIGHTNESS / 4);
      }
//      leds[iLED] = CHSV(iHue, 255, iV);
      leds[iLED] = CHSV((i * 32), 255, iV);
    }
    FastLED.show();  
}

void setup() {
  pinMode(ENCODER_CLOCK_PIN, INPUT);
  pinMode(ENCODER_DATA_PIN, INPUT);
  pinMode(ENCODER_SWITCH_PIN, INPUT_PULLUP);
  /* 
   * Read Pin A. Whatever state it’s in will reflect the last position.
  */
  iValLast = digitalRead(ENCODER_CLOCK_PIN);

  delay(1000);
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, LED_COLOR_MODEL>(leds, LED_NUMBER).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(LED_BRIGHTNESS);
  FastLED.show();
  
  Serial.begin(9600);
  Serial.println("Start");
  Serial.println(ENCODER_POS_PER_LED);

}

void loop() {
  /* BEGIN – Code for encoder */
  iVal = digitalRead(ENCODER_CLOCK_PIN);
  if (iVal != iValLast) { // Means the knob is rotating
    // The knob is rotating, we need to determine direction.
    // We do that by reading pin B.
    if (0 == iVal) { // aVal is false or 0 then proceed. This prevents double incrementation.
      int iDat = digitalRead(ENCODER_DATA_PIN);
      if (iDat != iVal) { // Means pin A Changed first, rotating clockwise.
        if (ENCODER_POS_MAX > iEncoderPos) {
          iEncoderPos++;
        }
        bCW = true;
      } else {  // Otherwise B changed first and moving CCW.
        if (ENCODER_POS_MIN < iEncoderPos) {
          iEncoderPos--;
        }
        bCW = false;
      }
      Serial.print("Encoder Position: ");
      Serial.println(iEncoderPos);
    }
  }
  iValLast = iVal;
  /* END – Code for encoder */
  int iLastLed = iEncoderPos / ENCODER_POS_PER_LED;
  if (iEncoderPos != iEncoderPosLast) {
    doSetShow();
    iEncoderPosLast = iEncoderPos;
  }

  if (!digitalRead(ENCODER_SWITCH_PIN)) {
    static unsigned long ulLastEntry = 0;
    unsigned long ulThisEntry = millis();
    // 200 ms debounce test.
    if (80 < (ulThisEntry - ulLastEntry)) {
      iHue += HUE_ORANGE;
      doSetShow();
      Serial.print("Hue Set: ");
      Serial.println(iHue);
    }
    ulLastEntry = ulThisEntry;
  }
}
