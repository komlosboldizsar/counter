#include <Arduino.h>
#include "DigitLedDisplay.h"

#define NOP_ADDR 0
#define DECODEMODE_ADDR 9
#define BRIGHTNESS_ADDR	10
#define SCANLIMIT_ADDR	11
#define SHUTDOWN_ADDR	12
#define DISPLAYTEST_ADDR 15


DigitLedDisplay::DigitLedDisplay(int numDisplays, int dinPin, int csPin, int clkPin) {

	_numDisplays = numDisplays;
	DIN_PIN = dinPin;
	CS_PIN = csPin;
	CLK_PIN = clkPin;
	
	pinMode(DIN_PIN, OUTPUT);
	pinMode(CS_PIN, OUTPUT);
	pinMode(CLK_PIN, OUTPUT);
	digitalWrite(CS_PIN, HIGH);

}

void DigitLedDisplay::initAll(int limit) {

	_digitLimit = limit;

	writeAll(DISPLAYTEST_ADDR, 0);
	writeAll(SCANLIMIT_ADDR, limit-1);
	writeAll(DECODEMODE_ADDR, 0);
	clearAll();
	writeAll(SHUTDOWN_ADDR, 1);

}

void DigitLedDisplay::setBrightAll(int brightness) {
	if ((brightness >= 0) && (brightness < 16))	
		writeAll(BRIGHTNESS_ADDR, brightness);
}
		
void DigitLedDisplay::onOne(int display) {
	writeOne(display, SHUTDOWN_ADDR, 0x01);
}

void DigitLedDisplay::onAll() {
	writeAll(SHUTDOWN_ADDR, 0x01);
}

void DigitLedDisplay::offOne(int display) {
	writeOne(display, SHUTDOWN_ADDR, 0x00);
}

void DigitLedDisplay::offAll() {
	writeAll(SHUTDOWN_ADDR, 0x00);
}

void DigitLedDisplay::clearOne(int display) {
  for (int i = 1; i <=_digitLimit; i++)
		writeOne(display, i, B00000000);
}

void DigitLedDisplay::clearAll() {
  for (int i = 1; i <=_digitLimit; i++)
		writeAll(i, B00000000);
}

void DigitLedDisplay::table(int display, byte address, int val, bool point) {
	byte tableValue;
	tableValue = *(charTable + val);
	if (point)
		tableValue |= B10000000;
	writeOne(display, address, tableValue);
}

void DigitLedDisplay::writeOne(int display, volatile byte address, volatile byte data) {
	digitalWrite(CS_PIN, LOW);
	for (int i = 0; i < display; i++)
		writeNop();
	shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, address);
	shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
	for (int i = display + 1; i < _numDisplays; i++)
		writeNop();
	digitalWrite(CS_PIN, HIGH);
}

void DigitLedDisplay::writeAll(volatile byte address, volatile byte data) {
	digitalWrite(CS_PIN, LOW);
	for (int i = 0; i < _numDisplays; i++) {
		shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, address);
		shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
	}
	digitalWrite(CS_PIN, HIGH);
}

void DigitLedDisplay::printDigit(int display, long number, bool point, byte startDigit) {
	String figure = String(number);
	int figureLength = figure.length();

	int parseInt;
	char str[2];
	for(int i = 0; i < figureLength; i++) {
		str[0] = figure[i];
		str[1] = '\0';
		parseInt = (int) strtol(str, NULL, 10);
		table(display, figureLength - i + startDigit, parseInt, (i == figureLength - 1));
	}
}

void DigitLedDisplay::writeNop() {
	shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, NOP_ADDR);
	shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, 0x00);
}
