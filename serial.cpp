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
        serialHandleCommand();
      } else {
        serialBuffer[serialBufferIdx++] = incomingByte;
      }
    }
  }
}

#define TOKEN_COUNT 3
#define TOKEN_MAXLENGTH 64

void serialHandleCommand() {

  if (serialBufferIdx == 0)
    return;

  char* serialBufferReader = serialBuffer;

  char tokens[TOKEN_COUNT][TOKEN_MAXLENGTH+1];
  for (int i = 0; i < TOKEN_COUNT; i++)
    memset(tokens[i], '\0', TOKEN_MAXLENGTH);

  int currentTokenIdx = 0;
  char* currentToken = tokens[0];
  int currentTokenLength = 0;

  while (*serialBufferReader == ' ')
    serialBufferReader++;

  bool prevIsSpace = false;
  while (*serialBufferReader != '\0')
  {
    if ((*serialBufferReader != ' ') || (currentTokenIdx == TOKEN_COUNT - 1)) {
        if (currentTokenLength < TOKEN_MAXLENGTH)
            *currentToken = *serialBufferReader;
        currentToken++;
        currentTokenLength++;
        prevIsSpace = false;
    } else {
        if (!prevIsSpace && (currentTokenIdx < TOKEN_COUNT - 1)) {
            currentToken = tokens[++currentTokenIdx];
            currentTokenLength = 0;
        }
        prevIsSpace = true;
    }
    serialBufferReader++;
  }

  if (strcmp(tokens[0], "reboot") == 0) {
    Serial.println("reboot");
  } else {
    settingsReceiveCommand(tokens[0], tokens[1], tokens[2]);
  }
  
  serialBufferIdx = 0;

}
