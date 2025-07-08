#ifndef DigitLedDisplay_h
#define DigitLedDisplay_h

#include <Arduino.h>

const static byte charTable [] = {
    B01111110,B00110000,B01101101,B01111001,B00110011,B01011011,B01011111,B01110000,B01111111,B01111011
};

class DigitLedDisplay
{

	private:
		int DIN_PIN;
		int CS_PIN;
		int CLK_PIN;
		int _numDisplays;
		int _digitLimit;
		void table(int display, byte address, int val, bool point);	
		void writeNop();

	public:
		DigitLedDisplay(int numDisplays, int dinPin, int csPin, int clkPin);
		void initAll(int limit);
		void setBrightAll(int brightness);

		void onOne(int display);
		void onAll();
		void offOne(int display);
		void offAll();
		void clearOne(int display);
		void clearAll();

		void writeOne(int display, byte address, byte data);
		void writeAll(byte address, byte data);
		void printDigit(int display, long number, bool point, byte startDigit = 0);
						
};

#endif	//DigitLedDisplay.h