#include "brightness.h"
#include "display.h"
#include "settings.h"
#include <Arduino.h>
#include <driver/adc.h>

bool brightnessDebug = false;
int brightnessDemo = 0;

void brightnessInit() {
  analogReadResolution(10);
}

int clamp(int value, int min, int max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

void brightnessSettingsFactory() {
  SETTINGS.BRIGHTNESS.sensormin = 150;
  SETTINGS.BRIGHTNESS.sensormax = 1000;
  SETTINGS.BRIGHTNESS.displaymin = 1;
  SETTINGS.BRIGHTNESS.displaymax = 15;
}

void brightnessSettingsDump() {
  settingsDumpPartStart(SETTING_BRIGHTNESS);
  settingsDumpValueInt(SETTING_BRIGHTNESS_SENSORMIN, SETTINGS.BRIGHTNESS.sensormin);
  settingsDumpValueInt(SETTING_BRIGHTNESS_SENSORMAX, SETTINGS.BRIGHTNESS.sensormax);
  settingsDumpValueInt(SETTING_BRIGHTNESS_DISPLAYMIN, SETTINGS.BRIGHTNESS.displaymin);
  settingsDumpValueInt(SETTING_BRIGHTNESS_DISPLAYMAX, SETTINGS.BRIGHTNESS.displaymax);
}

#define VALUE_SENSOR_MIN    1
#define VALUE_SENSOR_MAX    4400
#define VALUE_DISPLAY_MIN   1
#define VALUE_DISPLAY_MAX   15

bool brightnessReceiveCommand(const char* subCommand, const char* argument) {

  bool handled = false;
  
  int limit = clamp(SETTINGS.BRIGHTNESS.sensormax - 1, VALUE_SENSOR_MIN, VALUE_SENSOR_MAX);
  handled = handleSubcommandInt(SETTING_BRIGHTNESS, SETTING_BRIGHTNESS_SENSORMIN, &SETTINGS.BRIGHTNESS.sensormin, VALUE_SENSOR_MIN, limit, subCommand, argument);
  if (handled)
    return true;

  limit = clamp(SETTINGS.BRIGHTNESS.sensormin + 1, VALUE_SENSOR_MIN, VALUE_SENSOR_MAX);
  handled = handleSubcommandInt(SETTING_BRIGHTNESS, SETTING_BRIGHTNESS_SENSORMAX, &SETTINGS.BRIGHTNESS.sensormax, limit, VALUE_SENSOR_MAX, subCommand, argument);
  if (handled)
    return true;

  limit = clamp(SETTINGS.BRIGHTNESS.displaymax - 1, VALUE_DISPLAY_MIN, VALUE_DISPLAY_MAX);
  handled = handleSubcommandInt(SETTING_BRIGHTNESS, SETTING_BRIGHTNESS_DISPLAYMIN, &SETTINGS.BRIGHTNESS.displaymin, VALUE_DISPLAY_MIN, limit, subCommand, argument);
  if (handled)
    return true;

  limit = clamp(SETTINGS.BRIGHTNESS.displaymin + 1, VALUE_DISPLAY_MIN, VALUE_DISPLAY_MAX);
  handled = handleSubcommandInt(SETTING_BRIGHTNESS, SETTING_BRIGHTNESS_DISPLAYMAX, &SETTINGS.BRIGHTNESS.displaymax, limit, VALUE_DISPLAY_MAX, subCommand, argument);
  if (handled)
    return true;

  handled = handleSubcommandBool(SETTING_BRIGHTNESS, SETTING_BRIGHTNESS_DEBUG, &brightnessDebug, subCommand, argument);
  if (handled)
    return true;

  handled = handleSubcommandInt(SETTING_BRIGHTNESS, SETTING_BRIGHTNESS_DEMO, &brightnessDemo, 0, 15, subCommand, argument);
  if (handled)
    return true;

  return false;
  
}

#define BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE 16
#define BRIGHTNESS_ANALOG_VALUE_MAX_STEP 30

int brightnessCurrent = 512;
int brightnessAnalogValueHistory[BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE];
int brightnessAnalogValueHistoryIndex = 0;

uint8_t brightnessCalculate(bool enableDebug) {

  // Read and clamp
  int analogValue = analogRead(32);
  if (brightnessDebug && enableDebug) {
    Serial.print("[BRIGHNESS] Current sensor value: ");
    Serial.print(analogValue, DEC);
    Serial.print(", ");
  }
  analogValue = clamp(analogValue, SETTINGS.BRIGHTNESS.sensormin, SETTINGS.BRIGHTNESS.sensormax);

  // Store
  brightnessAnalogValueHistory[brightnessAnalogValueHistoryIndex++] = analogValue;
  if (brightnessAnalogValueHistoryIndex >= BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE)
    brightnessAnalogValueHistoryIndex = 0;

  // Average
  long analogValueSum = 0;
  for (uint8_t i = 0; i < BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE; i++)
    analogValueSum += brightnessAnalogValueHistory[i];
  int analogValueAvg = analogValueSum / BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE;
  if (brightnessDebug && enableDebug) {
    Serial.print("averaged sensor value: ");
    Serial.print(analogValueAvg, DEC);
    Serial.print(", ");
  }

  // Limit step
  int analogValueDiff = analogValueAvg - brightnessCurrent;
  if ((analogValueDiff > 0) && (analogValueDiff > BRIGHTNESS_ANALOG_VALUE_MAX_STEP))
    analogValueDiff = BRIGHTNESS_ANALOG_VALUE_MAX_STEP;
  if ((analogValueDiff < 0) && (analogValueDiff < -1 * BRIGHTNESS_ANALOG_VALUE_MAX_STEP))
    analogValueDiff = -1 * BRIGHTNESS_ANALOG_VALUE_MAX_STEP;

  brightnessCurrent += analogValueDiff;

  // Scale
  int brightnessDisplayDiff = brightnessCurrent - SETTINGS.BRIGHTNESS.sensormin;
  float brightnessPercent = (float)brightnessDisplayDiff / (SETTINGS.BRIGHTNESS.sensormax - SETTINGS.BRIGHTNESS.sensormin);
  float brightnessDisplayValue = (SETTINGS.BRIGHTNESS.displaymax - SETTINGS.BRIGHTNESS.displaymin) * brightnessPercent + SETTINGS.BRIGHTNESS.displaymin;
  float brightnessDisplayValueRounded = round(brightnessDisplayValue);
  if (brightnessDebug && enableDebug) {
    Serial.print("display value: ");
    if (brightnessDemo == 0) {
      Serial.print((int)brightnessDisplayValueRounded, DEC);
    } else {
      Serial.print(brightnessDemo, DEC);
      Serial.print(" (demo)");
    }
    Serial.println(".");
  }

  return (brightnessDemo == 0) ? brightnessDisplayValueRounded : brightnessDemo;

}

void brightnessUpdate(bool enableDebug) {
  uint8_t brightness = brightnessCalculate(enableDebug);
  displaySetBrightness(brightness);
}
