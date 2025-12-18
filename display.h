#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "settings.h"
#include <Arduino.h>

#define SETTING_DISPLAY                "display"
#define SETTING_DISPLAY_COUNT          "count"

struct SettingsDisplay {
  int count;
};

extern SettingsDisplay SETTINGS_DISPLAY;
extern SettingsManager SM_DISPLAY;

#define MAX_NUM_DISPLAYS     8
#define DIGITS_PER_DISPLAY   4
#define DISPLAY_IDX_ALL     -1
#define DIGIT_IDX_ALL       -1

enum DotForcing {
  DOT_NO_FORCE,
  DOT_FORCE_OFF,
  DOT_FORCE_ON
};

void displayInit();
void displayMainLoop();

bool displaySetDataNatural(int dispIdx, const char* data);
bool displaySetDataHex(int dispIdx, const char* data);
void displaySetData(int dispIdx, const char* data);

void displaySetDotForcing(int display, int digit, DotForcing df);
void displayClearDotForcing(int display = DISPLAY_IDX_ALL, int digit = DIGIT_IDX_ALL);

void displayClear(int dispIdx = DISPLAY_IDX_ALL);
void displaySetBrightness(uint8_t brightness);
void displaySetOnOff(uint8_t on);

#endif