#include "brightness.h"
#include "display.h"
#include <Arduino.h>

#define BRIGHTNESS_ANALOG_VALUE_MIN 150
#define BRIGHTNESS_ANALOG_VALUE_MAX 1000
#define BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE 16
#define BRIGHTNESS_ANALOG_VALUE_MAX_STEP 30

#define BRIGHTNESS_DISPLAY_MIN 1
#define BRIGHTNESS_DISPLAY_MAX 15

int brightnessCurrent = 512;
int brightnessAnalogValueHistory[BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE];
int brightnessAnalogValueHistoryIndex = 0;

uint8_t brightnessCalculate() {

  // Read
  int analogValue = analogRead(32);
  //Serial.print(analogValue, DEC);
  //Serial.print(" -- ");

  // Clamp
  if (analogValue < BRIGHTNESS_ANALOG_VALUE_MIN)
    analogValue = BRIGHTNESS_ANALOG_VALUE_MIN;
  if (analogValue > BRIGHTNESS_ANALOG_VALUE_MAX)
    analogValue = BRIGHTNESS_ANALOG_VALUE_MAX;

  // Store
  brightnessAnalogValueHistory[brightnessAnalogValueHistoryIndex++] = analogValue;
  if (brightnessAnalogValueHistoryIndex >= BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE)
    brightnessAnalogValueHistoryIndex = 0;

  // Average
  long analogValueSum = 0;
  for (uint8_t i = 0; i < BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE; i++)
    analogValueSum += brightnessAnalogValueHistory[i];
  int analogValueAvg = analogValueSum / BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE;
  //Serial.print(analogValueSum, DEC);
  //Serial.print(" -- ");

  // Limit step
  int analogValueDiff = analogValueAvg - brightnessCurrent;
  if ((analogValueDiff > 0) && (analogValueDiff > BRIGHTNESS_ANALOG_VALUE_MAX_STEP))
    analogValueDiff = BRIGHTNESS_ANALOG_VALUE_MAX_STEP;
  if ((analogValueDiff < 0) && (analogValueDiff < -1 * BRIGHTNESS_ANALOG_VALUE_MAX_STEP))
    analogValueDiff = -1 * BRIGHTNESS_ANALOG_VALUE_MAX_STEP;

  brightnessCurrent += analogValueDiff;
  //Serial.print(brightnessCurrent, DEC);
  //Serial.print(" -- ");

  // Scale
  int brightnessDisplayDiff = brightnessCurrent - BRIGHTNESS_ANALOG_VALUE_MIN;
  float brightnessPercent = (float)brightnessDisplayDiff / (BRIGHTNESS_ANALOG_VALUE_MAX - BRIGHTNESS_ANALOG_VALUE_MIN);
  float brightnessDisplayValue = (BRIGHTNESS_DISPLAY_MAX - BRIGHTNESS_DISPLAY_MIN) * brightnessPercent + BRIGHTNESS_DISPLAY_MIN;
  float brightnessDisplayValueRounded = round(brightnessDisplayValue);
  //Serial.print(brightnessDisplayValue, DEC);
  //Serial.print(" -- ");
  //Serial.print(brightnessDisplayValueRounded, DEC);
  //Serial.println(" -- ");

  return brightnessDisplayValueRounded;

}

void brightnessUpdate() {
  uint8_t brightness = brightnessCalculate();
  cli();
  displaySetBrightness(brightness);
  sei();
}