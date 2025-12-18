#include "display.h"
#include "settings.h"
#include "pins.h"
#include "utils.h"
#include <Arduino.h>
#include "DigitLedDisplay.h"

DigitLedDisplay display(MAX_NUM_DISPLAYS, DATA_DISP, ENABLE_DISP, CLK_DISP); // data, enable, clk

const static uint8_t SEGMENTS_NUMBERS[] = { 
  B01111110, B00110000, B01101101, B01111001, B00110011, // 0-4
  B01011011, B01011111, B01110000, B01111111, B01111011  // 5-9
};
const static uint8_t SEGMENTS_SPEC[] = {
  B00000000,    // nothing  " "
  B00000001     // dash     "-"
};
#define IMAGE_NOTHING   0
#define IMAGE_DASH      1

uint8_t segments[MAX_NUM_DISPLAYS][DIGITS_PER_DISPLAY];
bool dots[MAX_NUM_DISPLAYS][DIGITS_PER_DISPLAY];

void displayInit() {
  display.setBrightness(15);
  display.init(DIGITS_PER_DISPLAY);
  displayClear();
}

void displayMainLoop() {
  for (int digitIdx = 0; digitIdx < DIGITS_PER_DISPLAY; digitIdx++) {
    display.startWrite();
    for (int displayIdx = SETTINGS_DISPLAY.count-1; displayIdx >= 0; displayIdx--) {
      byte segNumber = segments[displayIdx][digitIdx];
      byte segDot = dots[displayIdx][digitIdx] ? 0x80 : 0x00;
      display.writeToAddress(digitIdx+1, segNumber|segDot);
    }
    display.endWrite();
  }
}

bool displaySetDataNatural(int dispIdx, const char* data) {

  bool valid = true;
  const char* dNumber = data;
  int numbers = 0;
  int decimals = 0;
  int firstDecimal = -1;
  bool negative = false;

  if (*data == '-') {
    negative = true;
    dNumber++;
  }

  const char* dPtr = dNumber;

  while (*dPtr != '\0') {
    if ((*dPtr >= '0') && (*dPtr <= '9')) {
      numbers++;
    } else if (*dPtr == '.') {
      decimals++;
      if (decimals == 1)
        firstDecimal = numbers;
    } else {
      valid = false;
    }
    dPtr++;
  }

  int reqDigits = numbers;
  if ((firstDecimal == 0) || negative)
    reqDigits++;

  if ((decimals > 1) || (reqDigits > 4))
    valid = false;
  if (!valid)
    return false;

  for (int i = 0; i < DIGITS_PER_DISPLAY - reqDigits; i++) {
    segments[dispIdx][i] = SEGMENTS_SPEC[IMAGE_NOTHING];
    dots[dispIdx][i] = false;
  }

  dPtr = dNumber;
  int position = 0;
  int number = (negative ? -1 : 0);
  while (position < reqDigits) {
    bool blindNumber = ((position == 0) && (firstDecimal == 0) && !negative);
    bool dp = ((number+1 == firstDecimal) || blindNumber);
    uint8_t segment = SEGMENTS_SPEC[IMAGE_NOTHING];
    if (negative && (number == -1)) {
      segment = SEGMENTS_SPEC[IMAGE_DASH];
    } else if (!blindNumber) {
      segment = SEGMENTS_NUMBERS[*dPtr - '0'];
    }
    segments[dispIdx][4 - reqDigits + position] = segment;
    dots[dispIdx][4 - reqDigits + position] = dp;
    if (number != -1) {
      dPtr++;
      if (dp && !blindNumber) {
        dPtr++;
      }
    } else if (dp) {
      dPtr++;
    }
    position++;
    number++;
  }

  return true;

}

bool displaySetDataHex(int dispIdx, const char* data) {

  // Validate
  if ((*data != 'h') && (*data != 'H'))
    return false;
  data++;
  const char* dPtr = data;
  for (int i = 0; i < DIGITS_PER_DISPLAY*2; i++) {
    bool number = ((*dPtr >= '0') && (*dPtr <= '9'));
    bool hexLower = ((*dPtr >= 'a') && (*dPtr <= 'f'));
    bool hexUpper = ((*dPtr >= 'A') && (*dPtr <= 'F'));
    if (!(number || hexLower || hexUpper))
      return false;
    dPtr++;
  }
  if (*dPtr != '\0')
    return false;

  dPtr = data;
  for (int digitIdx = 0; digitIdx < DIGITS_PER_DISPLAY; digitIdx++) {
    int value = hexChrToInt(*dPtr);
    value *= 16;
    dPtr++;
    value += hexChrToInt(*dPtr);
    dPtr++;
    segments[dispIdx][digitIdx] = value;
    dots[dispIdx][digitIdx] = false;
  }
  return true;

}

void displaySetData(int dispIdx, const char* data) {
  if ((*data == 'h') || (*data == 'H')) {
    displaySetDataHex(dispIdx, data);
    return;
  }
  displaySetDataNatural(dispIdx, data);
}

void displayClear(int dispIdx) {
  if (dispIdx == DISPLAY_IDX_ALL) {
    for (int displayIdx = 0; displayIdx < SETTINGS_DISPLAY.count; displayIdx++) {
      for (int segmentIdx = 0; segmentIdx < DIGITS_PER_DISPLAY; segmentIdx++) {
        segments[displayIdx][segmentIdx] = SEGMENTS_SPEC[IMAGE_NOTHING];
        dots[displayIdx][segmentIdx] = false;
      }
    }
    return;
  }
  if ((dispIdx < 0) || (dispIdx >= MAX_NUM_DISPLAYS))
    return;
  for (int segmentIdx = 0; segmentIdx < DIGITS_PER_DISPLAY; segmentIdx++) {
    segments[dispIdx][segmentIdx] = SEGMENTS_SPEC[IMAGE_NOTHING];
    dots[dispIdx][segmentIdx] = false;
  }
}

void displaySetBrightness(uint8_t brightness) {
  display.setBrightness(brightness);
}

void displaySetOnOff(uint8_t on) {
  if (on)
    display.on();
  else
    display.off();
}

/* Settings */
SettingsDisplay SETTINGS_DISPLAY;

SettingValueManagerInt SVM_DISPLAY_COUNT(SETTING_DISPLAY_COUNT, &SETTINGS_DISPLAY.count, 1, 1, MAX_NUM_DISPLAYS);

SettingValueManager* SM_DISPLAY_MEMBERS[] = { &SVM_DISPLAY_COUNT, NULL };
SettingsManager SM_DISPLAY(SETTING_DISPLAY, LONGTIME, SETTINGS_DATA(SETTINGS_DISPLAY), SM_DISPLAY_MEMBERS);
