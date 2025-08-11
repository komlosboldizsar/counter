#include "settings.h"
#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_SIZE 128

const char* SETTING_BOOL_YES = "yes";
const char* SETTING_BOOL_NO = "no";
const long INITIALIZED = 0xBEF1C2D3;
SettingsEEPROM SETTINGS;

void settingsInit() {
  EEPROM.begin(sizeof(SettingsEEPROM));
  EEPROM.get(0, SETTINGS);
  if (SETTINGS.initialized != INITIALIZED) {
    Serial.println("Found no settings, applying factory ones.");
    settingsFactory();
  }
}

void settingsSave() {
  EEPROM.put(0, SETTINGS);
  EEPROM.commit();
}

void settingsFactory() {
  deviceSettingsFactory();
  wifiSettingsFactory();
  mqttSettingsFactory();
  brightnessSettingsFactory();
  SETTINGS.initialized = INITIALIZED;
  Serial.println("Factory settings restored.");
  settingsSave();
}

void settingsDump() {
  Serial.println("**** SETTINGS DUMP ****");
  deviceSettingsDump();
  wifiSettingsDump();
  mqttSettingsDump();
  brightnessSettingsDump();
  Serial.println("**** SETTINGS DUMP END ****");
  Serial.println("");
}

#define HANDLE_COMMAND_START(CMD)         if (strcmp(mainCommand, CMD) == 0) {
#define HANDLE_COMMAND_END()                return; \
                                          }

#define FORWARD_COMMAND(CMD, HANDLER)     HANDLE_COMMAND_START(CMD) \
                                            HANDLER(subCommand, argument); \
                                          HANDLE_COMMAND_END()

void settingsReceiveCommand(const char* mainCommand, const char* subCommand, const char* argument) {
    
  HANDLE_COMMAND_START("dump")
    settingsDump();
  HANDLE_COMMAND_END()
  
  HANDLE_COMMAND_START("save")
    settingsSave();
  HANDLE_COMMAND_END()

  HANDLE_COMMAND_START("factory")
    settingsFactory();
  HANDLE_COMMAND_END()

  HANDLE_COMMAND_START("reset")
    ESP.restart();
  HANDLE_COMMAND_END()

  FORWARD_COMMAND(SETTING_DEVICE, deviceReceiveCommand)
  FORWARD_COMMAND(SETTING_WIFI, wifiReceiveCommand)
  FORWARD_COMMAND(SETTING_MQTT, mqttReceiveCommand)
  FORWARD_COMMAND(SETTING_BRIGHTNESS, brightnessReceiveCommand)

}

void settingsDumpPartStart(const char* partName) {
  char line[64];
  memset(line, '\0', 64);
  strcat(line, "_ ");
  strncat(line, partName, 16);
  strcat(line, ": ");
  Serial.println(line);
}

void settingsDumpValueString(const char* settingName, const char* value) {
  char line[64];
  memset(line, '\0', 64);
  strcat(line, "___ ");
  strncat(line, settingName, 16);
  strcat(line, ": ");
  uint8_t len1 = strlen(line);
  memset(line+len1, ' ', 24 - len1);
  strcat(line, "[");
  strncat(line, value, 32);
  strcat(line, "]");
  Serial.println(line);
}

void settingsDumpValueInt(const char* settingName, int value) {
  char valueStr[sizeof(int)*3+3];
  itoa(value, valueStr, 10);
  settingsDumpValueString(settingName, valueStr);
}

void settingsDumpValueBool(const char* settingName, bool value) {
  settingsDumpValueString(settingName, value ? SETTING_BOOL_YES : SETTING_BOOL_NO);
}

void settingsDoneValueString(const char* groupName, const char* settingName, const char* value) {
  char line[64];
  memset(line, '\0', 64);
  strncat(line, groupName, 16);
  strcat(line, ".");
  strncat(line, settingName, 16);
  strcat(line, " set to: ");
  strncat(line, value, 32);
  Serial.println(line);
}

void settingsDoneValueInt(const char* groupName, const char* settingName, int value) {
  char valueStr[sizeof(int)*3+3];
  itoa(value, valueStr, 10);
  settingsDoneValueString(groupName, settingName, valueStr);
}

void settingsDoneValueBool(const char* groupName, const char* settingName, bool value) {
  settingsDoneValueString(groupName, settingName, value ? SETTING_BOOL_YES : SETTING_BOOL_NO);
}

void settingsError(const char* groupName, const char* settingName, const char* errorMessage) {
  char line[64];
  memset(line, '\0', 64);
  strncat(line, groupName, 16);
  strcat(line, ".");
  strncat(line, settingName, 16);
  strcat(line, " setting error: ");
  strncat(line, errorMessage, 32);
  strcat(line, ".");
  Serial.println(line);
}

bool handleSubcommandString(const char* groupName, const char* settingName, char* target, int8_t minLength, int8_t maxLength, const char* subCommand, const char* argument) {

  if (strcmp(subCommand, settingName) != 0)
    return false;

  if ((minLength > -1) && (strlen(argument) < minLength)) {
    settingsError(groupName, settingName, "too short");
  } else if ((maxLength > -1) && (strlen(argument) > maxLength)) {
    settingsError(groupName, settingName, "too long");
  } else {
    strcpy(target, argument);
    settingsDoneValueString(groupName, settingName, argument);
  }
  return true;

}

bool handleSubcommandInt(const char* groupName, const char* settingName, int* target, int min, int max, const char* subCommand, const char* argument) {

  if (strcmp(subCommand, settingName) != 0)
    return false;

  bool negative = false;
  bool invalid = false;
  long value = 0;

  if (*argument == '-') {
    negative = true;
    argument++;
  } else if (*argument == '+') {
    argument++;
  }

  while (!invalid && (*argument != '\0')) {
    if ((*argument >= '0') && (*argument <= '9')) {
      value *= 10;
      value += *argument - '0';
      argument++;
    } else {
      invalid = true;
    }
  }

  if (negative)
    value *= -1;

  Serial.println(value, DEC);

  if (invalid) {
    settingsError(groupName, settingName, "invalid integer value");
  } else if (value < min) {
    settingsError(groupName, settingName, "too small");
  } else if (value > max) {
    settingsError(groupName, settingName, "too big");
  } else {
    *target = (int)value;
  }
  return true;

}

bool handleSubcommandBool(const char* groupName, const char* settingName, bool* target, const char* subCommand, const char* argument) {

  if (strcmp(subCommand, settingName) != 0)
    return false;

  if (strcmp(argument, SETTING_BOOL_YES) == 0) {
    *target = true;
    settingsDoneValueBool(groupName, settingName, true);
  } else if (strcmp(argument, SETTING_BOOL_NO) == 0) {
    *target = false;
    settingsDoneValueBool(groupName, settingName, false);
  } else {
    settingsError(groupName, settingName, "invalid value (yes/no)");
  }
  return true;

}