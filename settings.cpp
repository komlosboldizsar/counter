#include "settings.h"
#include "main.h"
#include "utils.h"
#include <Arduino.h>

#include "device.h"
#include "mywifi.h"
#include "mqtt.h"
#include "brightness.h"
#include "telnet.h"

SettingsManager* SETTINGS[] = {
  &SM_DEVICE,
  &SM_WIFI,
  &SM_MQTT,
  &SM_BRIGHTNESS,
  &SM_RT_BRIGHTNESS,
  &SM_TELNET
};

#define LOOP_SETTINGS()     for (int i = 0; i < sizeof(SETTINGS)/sizeof(SettingsManager*); i++)

void settingsInit() {
  SettingsManager::init();
}

void settingsMainLoop() {
  LOOP_SETTINGS()
    SETTINGS[i]->runtimeMainLoop();
}

void settingsLoad() {
  LOOP_SETTINGS()
    SETTINGS[i]->load();
}

void settingsEmptyOutputFunc(const char* message) { }

void settingsSave(void(*outputFunc)(const char* message)) {
  if (outputFunc == NULL)
    outputFunc = settingsEmptyOutputFunc;
  LOOP_SETTINGS()
    SETTINGS[i]->save();
  outputFunc("Settings saved.\r\n");
}

void settingsFactory(const char* categoryName, void(*outputFunc)(const char* message)) {
  if (outputFunc == NULL)
    outputFunc = settingsEmptyOutputFunc;
  if (strlen(categoryName) == 0) {
    LOOP_SETTINGS()
      SETTINGS[i]->factoryValues();
    outputFunc("Factory values restored for all settings.\r\n");
  } else {
    bool validCategory = false;
    LOOP_SETTINGS() {
      if (strcmp(categoryName, SETTINGS[i]->_name) == 0) {
          SETTINGS[i]->factoryValues();
      validCategory = true;
      }
    }
    if (validCategory) {
        outputFunc("Factory values restored for settings in category [");
        outputFunc(categoryName);
        outputFunc("]\r\n");
    } else {
        outputFunc("ERROR: UNKNOWN SETTINGS CATEGORY [");
        outputFunc(categoryName);
        outputFunc("]\r\n");
    }
  }

}

void settingsDump(const char* categoryName, void(*outputFunc)(const char* message)) {
  bool all = (strlen(categoryName) == 0);
  bool validCategory = false;
  outputFunc("**** SETTINGS DUMP ");
  if (!all) {
    outputFunc("[");
    outputFunc(categoryName);
    outputFunc("] ");
  }
  outputFunc("****\r\n");
  LOOP_SETTINGS() {
    if ((SETTINGS[i]->_type == LONGTIME) && (all || (strcmp(categoryName, SETTINGS[i]->_name) == 0))) {
      SETTINGS[i]->dumpValues(outputFunc);
	  validCategory = true;
	}
  }
  if (!all && !validCategory) {
    outputFunc("ERROR: UNKNOWN SETTINGS CATEGORY [");
    outputFunc(categoryName);
    outputFunc("]\r\n");
  }
  outputFunc("**** SETTINGS DUMP END ****\r\n");
  outputFunc("\r\n");
}

void settingsExport(const char* categoryName, void(*outputFunc)(const char* message)) {
  bool all = (strlen(categoryName) == 0);
  bool validCategory = false;
  outputFunc(">>>> SETTINGS EXPORT ");
  if (!all) {
    outputFunc("[");
    outputFunc(categoryName);
    outputFunc("] ");
  }
  outputFunc(">>>>\r\n");
  LOOP_SETTINGS() {
    if ((SETTINGS[i]->_type == LONGTIME) && (all || (strcmp(categoryName, SETTINGS[i]->_name) == 0))) {
      SETTINGS[i]->exportValues(outputFunc);
	  validCategory = true;
	}
  }
  if (!all && !validCategory) {
    outputFunc("ERROR: UNKNOWN SETTINGS CATEGORY [");
    outputFunc(categoryName);
    outputFunc("]\r\n");
  } else {
    outputFunc("save\r\n");
    outputFunc("reset\r\n");
  }
  outputFunc("<<<< SETTINGS EXPORT END <<<<\r\n");
  outputFunc("\r\n");
}

#define HANDLE_COMMAND_START(CMD)         if (!handled && (strcmp(mainCommand, CMD) == 0)) {
#define HANDLE_COMMAND_END()                handled = true; \
                                          }

void settingsReceiveCommand(const char* mainCommand, const char* subCommand, const char* argument, void(*outputFunc)(const char* message)) {
    
  bool handled = false;
  
  HANDLE_COMMAND_START("dump")
    settingsDump(subCommand, outputFunc);
  HANDLE_COMMAND_END()

  HANDLE_COMMAND_START("export")
    settingsExport(subCommand, outputFunc);
  HANDLE_COMMAND_END()
  
  HANDLE_COMMAND_START("save")
    settingsSave(outputFunc);
  HANDLE_COMMAND_END()

  HANDLE_COMMAND_START("factory")
    settingsFactory(subCommand, outputFunc);
  HANDLE_COMMAND_END()

  HANDLE_COMMAND_START("reset")
    outputFunc("Reset ESP...\r\n");
    ESP.restart();
  HANDLE_COMMAND_END()

  LOOP_SETTINGS() {
    if (strcmp(mainCommand, SETTINGS[i]->_name) == 0) {
	    handled = true;
      if (SETTINGS[i]->receiveCommand(subCommand, argument, outputFunc))
        break;
    }
  }
  
  if (!handled)
    outputFunc("ERROR: UNKNOWN COMMAND\r\n");  

}

#define TOKEN_COUNT 3
#define TOKEN_MAXLENGTH 64

void settingsReceiveLine(const char* line, void(*outputFunc)(const char* message)) {

  const char* lineReader = line;

  char tokens[TOKEN_COUNT][TOKEN_MAXLENGTH+1];
  for (int i = 0; i < TOKEN_COUNT; i++)
    memset(tokens[i], '\0', TOKEN_MAXLENGTH);

  int currentTokenIdx = 0;
  char* currentToken = tokens[0];
  int currentTokenLength = 0;

  while (*lineReader == ' ')
    lineReader++;

  bool prevIsSpace = false;
  while ((*lineReader != '\0') && (*lineReader != '\r') && (*lineReader != '\n'))
  {
    if ((*lineReader != ' ') || (currentTokenIdx == TOKEN_COUNT - 1)) {
      if (currentTokenLength < TOKEN_MAXLENGTH)
        *currentToken = *lineReader;
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
    lineReader++;
  }
  
  settingsReceiveCommand(tokens[0], tokens[1], tokens[2], outputFunc);

}

/* end settings...() methods */