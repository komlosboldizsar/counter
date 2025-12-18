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

void DigitLedDisplay::init(int limit) {
	_digitLimit = limit;
	writeAll(DISPLAYTEST_ADDR, 0);
	writeAll(SCANLIMIT_ADDR, limit-1);
	writeAll(DECODEMODE_ADDR, 0);
	clear();
	writeAll(SHUTDOWN_ADDR, 1);
}


void DigitLedDisplay::setNumDisplays(int numDisplays) {
	_numDisplays = numDisplays;
}

void DigitLedDisplay::setBrightness(int brightness) {
	if ((brightness >= 0) && (brightness < 16))	
		writeAll(BRIGHTNESS_ADDR, brightness);
}

void DigitLedDisplay::on() {
	writeAll(SHUTDOWN_ADDR, 0x01);
}

void DigitLedDisplay::off() {
	writeAll(SHUTDOWN_ADDR, 0x00);
}

void DigitLedDisplay::clear() {
  for (int i = 1; i <= _digitLimit; i++)
		writeAll(i, B00000000);
}

void DigitLedDisplay::startWrite() {
	digitalWrite(CS_PIN, LOW);
}

void DigitLedDisplay::endWrite() {
	digitalWrite(CS_PIN, HIGH);
}

void DigitLedDisplay::writeToAddress(byte address, byte data) {
	shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, address);
	shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
}

void DigitLedDisplay::writeOne(int display, volatile byte address, volatile byte data) {
	startWrite();
	for (int i = display + 1; i < _numDisplays; i++)
		writeNop();
	writeToAddress(address, data);
	for (int i = 0; i < display; i++)
		writeNop();
	endWrite();
}

void DigitLedDisplay::writeAll(volatile byte address, volatile byte data) {
	startWrite();
	for (int i = 0; i < _numDisplays; i++)
		writeToAddress(address, data);
	endWrite();
}

void DigitLedDisplay::writeNop() {
	writeToAddress(NOP_ADDR, 0x00);
}
