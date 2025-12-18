#ifndef DigitLedDisplay_h
#define DigitLedDisplay_h

#include <Arduino.h>

class DigitLedDisplay
{

	private:
		int DIN_PIN;
		int CS_PIN;
		int CLK_PIN;
		int _numDisplays;
		int _digitLimit;
		void writeNop();

	public:
		DigitLedDisplay(int numDisplays, int dinPin, int csPin, int clkPin);
		void setNumDisplays(int numDisplays);
		void init(int limit);

		void setBrightness(int brightness);
		void on();
		void off();
		void clear();

		void startWrite();
		void endWrite();
		void writeToAddress(byte address, byte data);
		void writeOne(int display, byte address, byte data);
		void writeAll(byte address, byte data);
						
};

#endif	//DigitLedDisplay.h