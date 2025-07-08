#include "display.h"
#include "pins.h"
#include <Arduino.h>
#include "DigitLedDisplay.h"

#define DISPLAY_NOTHING 0b00000000
#define DISPLAY_DASH    0b00000001

DigitLedDisplay display(1, DATA_DISP, ENABLE_DISP, CLK_DISP); // data, enable, clk

void displayInit() {
  display.setBrightAll(15);
  display.initAll(4);
  displayClear();
}

void displayDigits(int dispIdx, uint8_t a, bool aDp, uint8_t b, bool bDp, uint8_t c, bool cDp, uint8_t d, bool dDp) {
  display.printDigit(dispIdx, a, aDp, 0);
  display.printDigit(dispIdx, b, bDp, 1);
  display.printDigit(dispIdx, c, cDp, 2);
  display.printDigit(dispIdx, d, dDp, 3);
}

void displayDashes(int dispIdx, int count) {
  for (int i = 0; i < 4; i++)
    display.writeOne(dispIdx, i+1, (i < count) ? DISPLAY_DASH : DISPLAY_NOTHING);
}

void displayClear() {
  display.clearAll();
}

void displaySetBrightness(uint8_t brightness) {
  display.setBrightAll(brightness);
}