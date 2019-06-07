/*
 * Copyright (c) 2019 Donald Purnhagen
 * 
 * ToDo: Write documentation!
 */

#include <FastLED.h>

#define SERIAL_DEBUG

#define HUE_MODE_SINGLE 0
#define HUE_MODE_MULTI 1
byte iHueMode = HUE_MODE_SINGLE;

/* BEGIN - LED Ring Variables */
#define LED_DATA_PIN 5
#define LED_NUMBER 8
#define LED_BRIGHTNESS_INC 16
#define LED_BRIGHTNESS_MAX 239 
#define LED_BRIGHTNESS_MIN 63 
#define LED_TYPE WS2812B
#define LED_COLOR_MODEL GRB
// Initial brightness.
byte iBrightness = 127;
byte iHue = HUE_ORANGE;
// Evenly divide the hue range: 256 / 8 = 32
static const byte iHueAdjust = (256 / LED_NUMBER);
CRGB leds[LED_NUMBER];

/* BEGIN – Encoder Variables */
#define ENCODER_CLOCK_PIN 4
#define ENCODER_DATA_PIN 3
#define ENCODER_SWITCH_PIN 7
#define ENCODER_POS_MIN 1
#define ENCODER_POS_MAX 20
// Keep track of encoder position value.
byte iEncoderPos = 1;
byte iEncoderPosLast = 1;
byte iVal;
byte iValLast;
boolean bCW;
// Keep track of encoder switch state.
boolean bPressed = false;
unsigned long ulPressedAt = 0;

void doSetShow() {
    byte iLastLed = map(iEncoderPos, ENCODER_POS_MIN, ENCODER_POS_MAX, 1, LED_NUMBER);
    for (int i = 0; i < LED_NUMBER; i++) {
      byte iV = 0;
      if (i < iLastLed) {
        switch (iHueMode) {
        case HUE_MODE_SINGLE:
          iV = map(i, 0, LED_NUMBER, LED_BRIGHTNESS_MIN, LED_BRIGHTNESS_MAX);
          break;
        case HUE_MODE_MULTI:
          iV = iBrightness;
          break;
        }
      }
      switch (iHueMode) {
      case HUE_MODE_SINGLE:
        leds[map(i, 0, LED_NUMBER - 1, LED_NUMBER - 1, 0)] = CHSV(iHue, 255, iV);
        break;
      case HUE_MODE_MULTI:
        leds[map(i, 0, LED_NUMBER - 1, LED_NUMBER - 1, 0)] = CHSV((i * iHueAdjust), 255, iV);
        break;
      }
    }
    FastLED.show();  
#if defined (SERIAL_DEBUG)
    Serial.print(iEncoderPos);
    Serial.print(",");
    Serial.print(iLastLed);
    Serial.print(",");
    Serial.println(iBrightness);
#endif //defined (SERIAL_DEBUG)
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
  FastLED.setBrightness(iBrightness);

#if defined (SERIAL_DEBUG)
  Serial.begin(9600);
  Serial.println("EncoderPos,LastLed,Brightness");  
#endif //defined (SERIAL_DEBUG)

  doSetShow();
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
    }
  }
  iValLast = iVal;
  /* END – Code for encoder */
  if (iEncoderPos != iEncoderPosLast) {
    doSetShow();
    iEncoderPosLast = iEncoderPos;
  }

  /*
   * Still need to work on debouncing the switch. 
   * I think I need to maintain the time of release as well.
   */
  if (!digitalRead(ENCODER_SWITCH_PIN)) {
    // Switch depressed.
    if (!bPressed) {
      bPressed = true;
      ulPressedAt = millis();
#if defined (SERIAL_DEBUG)
      Serial.print("Pressed: ");
      Serial.println(ulPressedAt);
#endif //defined (SERIAL_DEBUG)
    }
  }
  else {
    if (bPressed) {
      unsigned long ulDuration = (millis() - ulPressedAt);
      if (999 < ulDuration) {
        iHueMode = ++iHueMode % 2;
#if defined (SERIAL_DEBUG)
        Serial.print("New Mode: ");
        Serial.println(iHueMode);
#endif //defined (SERIAL_DEBUG)
        doSetShow();
      }
      else if (79 < ulDuration) {
        switch (iHueMode) {
        case HUE_MODE_SINGLE:
          iHue += HUE_ORANGE;
          break;
        case HUE_MODE_MULTI:
          iBrightness += LED_BRIGHTNESS_INC;
          if ((LED_BRIGHTNESS_MIN > iBrightness) || (LED_BRIGHTNESS_MAX < iBrightness)) {
            iBrightness = LED_BRIGHTNESS_MIN;
          }
          FastLED.setBrightness(iBrightness);
          break;
        }
        doSetShow();
      }
      bPressed = false;
    }
  }

}
