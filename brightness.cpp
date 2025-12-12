#include "brightness.h"
#include "display.h"
#include "settings.h"
#include "mqtt.h"
#include <Arduino.h>
#include <driver/adc.h>

void brightnessInit() {
  analogReadResolution(10);
}

/* Utility */
int clamp(int value, int min, int max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

/* Setting custom - brightness.{sensor/display}{min/max} */
bool brightnessDebug = false;

#define VALUE_SENSOR_MIN    1
#define VALUE_SENSOR_MAX    4400
#define VALUE_DISPLAY_MIN   1
#define VALUE_DISPLAY_MAX   15

extern SettingValueManagerInt SVM_BRIGHTNESS_SENSORMIN, SVM_BRIGHTNESS_SENSORMAX, SVM_BRIGHTNESS_DISPLAYMIN, SVM_BRIGHTNESS_DISPLAYMAX;

void svm_brightness_Xminmax_getlimits(SettingValueManager* svm, int* min, int* max) {
  if (svm == &SVM_BRIGHTNESS_SENSORMIN) {
    *min = VALUE_SENSOR_MIN;
	  *max = clamp(SETTINGS_BRIGHTNESS.sensormax - 1, VALUE_SENSOR_MIN, VALUE_SENSOR_MAX);
	  return;
  }
  if (svm == &SVM_BRIGHTNESS_SENSORMAX) {
    *min = clamp(SETTINGS_BRIGHTNESS.sensormin + 1, VALUE_SENSOR_MIN, VALUE_SENSOR_MAX);
	  *max = VALUE_SENSOR_MAX;
	  return;
  }
  if (svm == &SVM_BRIGHTNESS_DISPLAYMIN) {
    *min = VALUE_DISPLAY_MIN;
	  *max = clamp(SETTINGS_BRIGHTNESS.displaymax - 1, VALUE_DISPLAY_MIN, VALUE_DISPLAY_MAX);
	  return;
  }
  if (svm == &SVM_BRIGHTNESS_DISPLAYMAX) {
    *min = clamp(SETTINGS_BRIGHTNESS.displaymin + 1, VALUE_DISPLAY_MIN, VALUE_DISPLAY_MAX);
	  *max = VALUE_DISPLAY_MAX;
	  return;
  }
}

void svm_brightness_Xminmax_initializer(SettingValueManager* svm)  {
  ((SettingValueManagerInt*)svm)->_hookGetLimits = svm_brightness_Xminmax_getlimits;
}

/* Setting custom - brightness.manual */
void svm_brightness_manual_afterset(SettingValueManager* svm, const char* argument, void(*outputFunc)(const char* message)) {
  RUNTIME_BRIGHTNESS.isAuto = false;
  Serial.println("Brightness mode set to manual.");
}

void svm_brightness_manual_initializer(SettingValueManager* svm)  {
  svm->_hookAfterSet = svm_brightness_manual_afterset;
}

/* Settings */
SettingsBrightness SETTINGS_BRIGHTNESS;

SettingValueManagerInt SVM_BRIGHTNESS_SENSORMIN(SETTING_BRIGHTNESS_SENSORMIN, &SETTINGS_BRIGHTNESS.sensormin, 150, VALUE_SENSOR_MIN, VALUE_SENSOR_MAX, svm_brightness_Xminmax_initializer);
SettingValueManagerInt SVM_BRIGHTNESS_SENSORMAX(SETTING_BRIGHTNESS_SENSORMAX, &SETTINGS_BRIGHTNESS.sensormax, 1000, VALUE_SENSOR_MIN, VALUE_SENSOR_MAX, svm_brightness_Xminmax_initializer);
SettingValueManagerInt SVM_BRIGHTNESS_DISPLAYMIN(SETTING_BRIGHTNESS_DISPLAYMIN, &SETTINGS_BRIGHTNESS.displaymin, 1, VALUE_DISPLAY_MIN, VALUE_DISPLAY_MAX, svm_brightness_Xminmax_initializer);
SettingValueManagerInt SVM_BRIGHTNESS_DISPLAYMAX(SETTING_BRIGHTNESS_DISPLAYMAX, &SETTINGS_BRIGHTNESS.displaymax, 15, VALUE_DISPLAY_MIN, VALUE_DISPLAY_MAX, svm_brightness_Xminmax_initializer);

SettingValueManager* SM_BRIGHTNESS_MEMBERS[] = { &SVM_BRIGHTNESS_SENSORMIN, &SVM_BRIGHTNESS_SENSORMAX, &SVM_BRIGHTNESS_DISPLAYMIN, &SVM_BRIGHTNESS_DISPLAYMAX, NULL };
SettingsManager SM_BRIGHTNESS(SETTING_BRIGHTNESS, LONGTIME, SETTINGS_DATA(SETTINGS_BRIGHTNESS), SM_BRIGHTNESS_MEMBERS);

/* Runtime */
RuntimeBrightness RUNTIME_BRIGHTNESS;

SettingValueManagerInt SVM_BRIGHTNESS_MANUAL(SETTING_BRIGHTNESS_MANUAL, &RUNTIME_BRIGHTNESS.manual, 7, 1, 15, svm_brightness_manual_initializer);
SettingValueManagerBool SVM_BRIGHTNESS_AUTO(SETTING_BRIGHTNESS_AUTO, &RUNTIME_BRIGHTNESS.isAuto, true);
SettingValueManagerBool SVM_BRIGHTNESS_SWITCH(SETTING_BRIGHTNESS_SWITCH, &RUNTIME_BRIGHTNESS.sw, true);
SettingValueManagerBool SVM_BRIGHTNESS_DEBUG(SETTING_BRIGHTNESS_DEBUG, &brightnessDebug, false, svm_initializer_hidden);

SettingValueManager* SM_RT_BRIGHTNESS_MEMBERS[] = { &SVM_BRIGHTNESS_MANUAL, &SVM_BRIGHTNESS_AUTO, &SVM_BRIGHTNESS_SWITCH, &SVM_BRIGHTNESS_DEBUG, NULL };
SettingsManager SM_RT_BRIGHTNESS(SETTING_BRIGHTNESS, RUNTIME, SETTINGS_DATA(RUNTIME_BRIGHTNESS), SM_RT_BRIGHTNESS_MEMBERS);

/* Calculation */

#define BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE 16
#define BRIGHTNESS_ANALOG_VALUE_MAX_STEP 30

int brightnessCurrent = 512;
int brightnessAnalogValueHistory[BRIGHTNESS_ANALOG_VALUE_HISTORY_SIZE];
int brightnessAnalogValueHistoryIndex = 0;

uint8_t brightnessPrevious = -1;
uint8_t brightnessCalculateCounter = 0;

uint8_t brightnessCalculate(bool enableDebug) {

  // Read and clamp
  int analogValue = analogRead(32);
  if (brightnessDebug && enableDebug) {
    Serial.print("[BRIGHNESS] Current sensor value: ");
    Serial.print(analogValue, DEC);
    Serial.print(", ");
  }
  analogValue = clamp(analogValue, SETTINGS_BRIGHTNESS.sensormin, SETTINGS_BRIGHTNESS.sensormax);

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

  if ((brightnessCalculateCounter & 0xFF) == 0xFF) {
    mqttNotifyIlluminanceChanged(analogValueAvg);
    brightnessCalculateCounter = 0;
  }

  // Limit step
  int analogValueDiff = analogValueAvg - brightnessCurrent;
  if ((analogValueDiff > 0) && (analogValueDiff > BRIGHTNESS_ANALOG_VALUE_MAX_STEP))
    analogValueDiff = BRIGHTNESS_ANALOG_VALUE_MAX_STEP;
  if ((analogValueDiff < 0) && (analogValueDiff < -1 * BRIGHTNESS_ANALOG_VALUE_MAX_STEP))
    analogValueDiff = -1 * BRIGHTNESS_ANALOG_VALUE_MAX_STEP;

  brightnessCurrent += analogValueDiff;

  // Scale
  int brightnessDisplayDiff = brightnessCurrent - SETTINGS_BRIGHTNESS.sensormin;
  float brightnessPercent = (float)brightnessDisplayDiff / (SETTINGS_BRIGHTNESS.sensormax - SETTINGS_BRIGHTNESS.sensormin);
  float brightnessDisplayValue = (SETTINGS_BRIGHTNESS.displaymax - SETTINGS_BRIGHTNESS.displaymin) * brightnessPercent + SETTINGS_BRIGHTNESS.displaymin;
  float brightnessDisplayValueRounded = round(brightnessDisplayValue);
  if (brightnessDebug && enableDebug) {
    Serial.print("display value: ");
    if (RUNTIME_BRIGHTNESS.isAuto) {
      Serial.print((int)brightnessDisplayValueRounded, DEC);
      Serial.print(" (auto)");
    } else {
      Serial.print(RUNTIME_BRIGHTNESS.manual, DEC);
      Serial.print(" (manual)");
    }
    Serial.println(".");
  }
  
  brightnessCalculateCounter++;

  return RUNTIME_BRIGHTNESS.isAuto ? brightnessDisplayValueRounded : RUNTIME_BRIGHTNESS.manual;

}

void brightnessUpdate(bool enableDebug) {
  uint8_t brightness = brightnessCalculate(enableDebug);
  displaySetBrightness(brightness);
  displaySetOnOff(RUNTIME_BRIGHTNESS.sw);
  if (brightness != brightnessPrevious)
    mqttNotifyBrightnessChanged(brightness);
  brightnessPrevious = brightness;
}
