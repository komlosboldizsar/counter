#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Arduino.h>

void displayInit();
void displayDigits(int dispIdx, uint8_t a, bool aDp, uint8_t b, bool bDp, uint8_t c, bool cDp, uint8_t d, bool dDp);
void displayDashes(int display, int count);
void displayClear();
void displaySetBrightness(uint8_t brightness);

#endif