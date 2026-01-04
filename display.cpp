#include "display.h"
#include "settings.h"
#include "pins.h"
#include "utils.h"
#include "mqtt.h"
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

DisplayData displayData[MAX_NUM_DISPLAYS];

void displayInit() {
  for (int i = 0; i < MAX_NUM_DISPLAYS; i++) {
    displayData[i].blink = false;
    displayData[i].blinkSpeed = DBS_SINGLE;
    displayData[i].blinkPhase = DBP_0;
    displayData[i].blinkDutyCycle = DBDC_50;
  }
  display.setBrightness(15);
  display.init(DIGITS_PER_DISPLAY);
  displayClear();
}

uint8_t displayBlinkCounter = 0;

void displayMainLoop() {

  bool displayOn[MAX_NUM_DISPLAYS];

  for (int displayIdx = 0; displayIdx < SETTINGS_DISPLAY.count; displayIdx++) {
    if (displayData[displayIdx].blink) {
      // speed
      uint8_t currentBlinkCounter = (displayBlinkCounter >> (3 + (int)displayData[displayIdx].blinkSpeed));
      currentBlinkCounter &= 0b11; // mod 4
      // phase
      currentBlinkCounter += (int)displayData[displayIdx].blinkPhase;
      currentBlinkCounter &= 0b11; // mod 4
      // duty
      displayOn[displayIdx] = (currentBlinkCounter < ((int)displayData[displayIdx].blinkDutyCycle + 1));
    } else {
      displayOn[displayIdx] = true;
    }
  }

  for (int digitIdx = 0; digitIdx < DIGITS_PER_DISPLAY; digitIdx++) {
    display.startWrite();
    for (int displayIdx = SETTINGS_DISPLAY.count-1; displayIdx >= 0; displayIdx--) {
      byte seg = displayData[displayIdx].segments[digitIdx];
      seg &= displayOn[displayIdx] ? 0xFF : 0x00;
      DotForcing df = displayData[displayIdx].dotForcing[digitIdx];
      if (df == DOT_FORCE_OFF)
        seg &= 0x7F;
      else if (df == DOT_FORCE_ON)
        seg |= 0x80;
      display.writeToAddress(digitIdx+1, seg);
    }
    display.endWrite();
  }

  displayBlinkCounter++;

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

  for (int i = 0; i < DIGITS_PER_DISPLAY - reqDigits; i++)
    displayData[dispIdx].segments[i] = SEGMENTS_SPEC[IMAGE_NOTHING];

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
    displayData[dispIdx].segments[4 - reqDigits + position] = segment | (dp << 7);
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
    displayData[dispIdx].segments[digitIdx] = value;
  }
  return true;

}

void displaySetData(int dispIdx, const char* data) {

  bool dataOk = false;

  if ((*data == 'h') || (*data == 'H')) {
    dataOk = displaySetDataHex(dispIdx, data);
  } else {
    dataOk = displaySetDataNatural(dispIdx, data);
  }

  if (dataOk) {
    strcpy(displayData[dispIdx].text, data);
    mqttNotifyDisplayTextChanged(dispIdx);
  }

}

void displaySetBlink(int dispIdx, bool blink) {
  displayData[dispIdx].blink = blink;
  mqttNotifyDisplayBlinkChanged(dispIdx);
}

void displaySetBlinkSpeed(int dispIdx, DisplayBlinkSpeed blinkSpeed) {
displayData[dispIdx].blinkSpeed = blinkSpeed;
  mqttNotifyDisplayBlinkSpeedChanged(dispIdx);
}

void displaySetBlinkPhase(int dispIdx, DisplayBlinkPhase blinkPhase) {
  displayData[dispIdx].blinkPhase = blinkPhase;
  mqttNotifyDisplayBlinkPhaseChanged(dispIdx);
}

void displaySetBlinkDutyCycle(int dispIdx, DisplayBlinkDutyCycle blinkDutyCycle) {
  displayData[dispIdx].blinkDutyCycle = blinkDutyCycle;
  mqttNotifyDisplayBlinkDutyCycleChanged(dispIdx);
}

void displaySetDotForcing(int display, int digit, DotForcing df) {
  displayData[display].dotForcing[digit] = df;
}

void displayClearDotForcing(int display, int digit) {
  if (display == DISPLAY_IDX_ALL) {
    for (int i = 0; i < SETTINGS_DISPLAY.count; i++)
      displayClearDotForcing(i, digit);
    return;
  }
  if (digit == DIGIT_IDX_ALL) {
    for (int i = 0; i < DIGITS_PER_DISPLAY; i++)
      displayClearDotForcing(display, i);
    return;
  }
  displayData[display].dotForcing[digit] = DOT_NO_FORCE;
}

void displayClear(int dispIdx) {
  if (dispIdx == DISPLAY_IDX_ALL) {
    for (int displayIdx = 0; displayIdx < SETTINGS_DISPLAY.count; displayIdx++)
      for (int segmentIdx = 0; segmentIdx < DIGITS_PER_DISPLAY; segmentIdx++)
        displayData[displayIdx].segments[segmentIdx] = SEGMENTS_SPEC[IMAGE_NOTHING];
    return;
  }
  if ((dispIdx < 0) || (dispIdx >= MAX_NUM_DISPLAYS))
    return;
  for (int segmentIdx = 0; segmentIdx < DIGITS_PER_DISPLAY; segmentIdx++)
    displayData[dispIdx].segments[segmentIdx] = SEGMENTS_SPEC[IMAGE_NOTHING];
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
