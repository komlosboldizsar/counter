#include "serial.h"
#include <Arduino.h>
#include "settings.h"

char serialBuffer[64];
int serialBufferIdx = 0;

void serialInit() {
  Serial.begin(115200);
}

void serialRead() {
  while (Serial.available() > 0) {
    if (serialBufferIdx > 62) {
      Serial.println("Command too long!");
    } else {
      char incomingByte = Serial.read();
      Serial.print(incomingByte);
      if (incomingByte == '\r' || incomingByte == '\n') {
        serialBuffer[serialBufferIdx++] = '\0';
        settingsReceiveLine(serialBuffer, serialWrite);
		serialBufferIdx = 0;
      } else {
        serialBuffer[serialBufferIdx++] = incomingByte;
      }
    }
  }
}

void serialWrite(const char* message) {
  Serial.print(message);
}

