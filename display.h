#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Arduino.h>

void displayInit();
void displayDigits(int dispIdx, uint8_t a, bool aDp, uint8_t b, bool bDp, uint8_t c, bool cDp, uint8_t d, bool dDp);
void displayDigit(int dispIdx, int pos, uint8_t a, bool aDp);
void displayDashes(int display, int count);
void displayClear(int dispIdx = -1);
void displaySetBrightness(uint8_t brightness);
void displaySetOnOff(uint8_t on);

#endif