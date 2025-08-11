#ifndef BRIGHTNESS_H_
#define BRIGHTNESS_H_

#include <Arduino.h>

struct SettingsBrightness {
  int sensormin;
  int sensormax;
  int displaymin;
  int displaymax;
};

#define SETTING_BRIGHTNESS                "brightness"
#define SETTING_BRIGHTNESS_SENSORMIN      "sensormin"
#define SETTING_BRIGHTNESS_SENSORMAX      "sensormax"
#define SETTING_BRIGHTNESS_DISPLAYMIN     "displaymin"
#define SETTING_BRIGHTNESS_DISPLAYMAX     "displaymax"
#define SETTING_BRIGHTNESS_DEBUG          "debug"
#define SETTING_BRIGHTNESS_DEMO           "demo"

void brightnessInit();
void brightnessSettingsFactory();
void brightnessSettingsDump();
bool brightnessReceiveCommand(const char* subCommand, const char* argument);
uint8_t brightnessCalculate(bool enableDebug = false);
void brightnessUpdate(bool enableDebug = false);

#endif