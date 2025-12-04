#ifndef BRIGHTNESS_H_
#define BRIGHTNESS_H_

#include "settings.h"
#include <Arduino.h>

#define SETTING_BRIGHTNESS                "brightness"
#define SETTING_BRIGHTNESS_SENSORMIN      "sensormin"
#define SETTING_BRIGHTNESS_SENSORMAX      "sensormax"
#define SETTING_BRIGHTNESS_DISPLAYMIN     "displaymin"
#define SETTING_BRIGHTNESS_DISPLAYMAX     "displaymax"
#define SETTING_BRIGHTNESS_AUTO           "auto"
#define SETTING_BRIGHTNESS_MANUAL         "manual"
#define SETTING_BRIGHTNESS_SWITCH         "switch"
#define SETTING_BRIGHTNESS_DEBUG          "debug"

struct SettingsBrightness {
  int sensormin;
  int sensormax;
  int displaymin;
  int displaymax;
};

extern SettingsBrightness SETTINGS_BRIGHTNESS;
extern SettingsManager SM_BRIGHTNESS;

struct RuntimeBrightness {
  int manual;
  bool isAuto;
  bool sw;
};

extern RuntimeBrightness RUNTIME_BRIGHTNESS;
extern SettingsManager SM_RT_BRIGHTNESS;

void brightnessInit();
uint8_t brightnessCalculate(bool enableDebug = false);
void brightnessUpdate(bool enableDebug = false);

#endif