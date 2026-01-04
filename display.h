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
#define DISPLAY_TEXT_MAXLEN  10

enum DotForcing {
  DOT_NO_FORCE,
  DOT_FORCE_OFF,
  DOT_FORCE_ON
};

enum DisplayBlinkSpeed {
  DBS_DOUBLE,
  DBS_SINGLE,
  DBS_HALF,
  DBS___end
};

enum DisplayBlinkPhase {
  DBP_0,
  DBP_90,
  DBP_180,
  DBP_270,
  DBP___end
};

enum DisplayBlinkDutyCycle {
  DBDC_25,
  DBDC_50,
  DBDC_75,
  DBDC___end
};

struct DisplayData {
  char text[DISPLAY_TEXT_MAXLEN+1];
  uint8_t segments[DIGITS_PER_DISPLAY];
  DotForcing dotForcing[DIGITS_PER_DISPLAY];
  bool blink;
  DisplayBlinkSpeed blinkSpeed;
  DisplayBlinkPhase blinkPhase;
  DisplayBlinkDutyCycle blinkDutyCycle;
};

extern DisplayData displayData[MAX_NUM_DISPLAYS];

void displayInit();
void displayMainLoop();

bool displaySetDataNatural(int dispIdx, const char* data);
bool displaySetDataHex(int dispIdx, const char* data);
void displaySetData(int dispIdx, const char* data);

void displaySetBlink(int dispIdx, bool blink);
void displaySetBlinkSpeed(int dispIdx, DisplayBlinkSpeed blinkSpeed);
void displaySetBlinkPhase(int dispIdx, DisplayBlinkPhase blinkPhase);
void displaySetBlinkDutyCycle(int dispIdx, DisplayBlinkDutyCycle blinkDutyCycle);

void displaySetDotForcing(int display, int digit, DotForcing df);
void displayClearDotForcing(int display = DISPLAY_IDX_ALL, int digit = DIGIT_IDX_ALL);

void displayClear(int dispIdx = DISPLAY_IDX_ALL);
void displaySetBrightness(uint8_t brightness);
void displaySetOnOff(uint8_t on);

#endif