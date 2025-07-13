#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <Arduino.h>

#include "mywifi.h"
#include "device.h"
#include "mqtt.h"

struct SettingsEEPROM {
  long initialized;
  SettingsWifi WIFI;
  char _wifi_placeholder[256-sizeof(SettingsWifi)];
  SettingsDevice DEVICE;
  char _device_placeholder[256-sizeof(SettingsDevice)];
  SettingsMqtt MQTT;
  char _mqtt_placeholder[256-sizeof(SettingsMqtt)];
};

extern SettingsEEPROM SETTINGS;

void settingsInit();
void settingsSave();
void settingsFactory();
void settingsReceiveCommand(const char* mainCommand, const char* subCommand, const char* argument);

void settingsDump();
void settingsDumpPartStart(const char* partName);
void settingsDumpValueString(const char* settingName, const char* value);
void settingsDumpValueInt(const char* settingName, int value);
void settingsDumpValueBool(const char* settingName, bool value);

void settingsDoneValueString(const char* groupName, const char* settingName, const char* value);
void settingsDoneValueInt(const char* groupName, const char* settingName, const char* value);
void settingsDoneValueBool(const char* groupName, const char* settingName, bool value);

void settingsError(const char* groupName, const char* settingName, const char* errorMessage);

bool handleSubcommandString(const char* groupName, const char* settingName, char* target, int8_t minLength, int8_t maxLength, const char* subCommand, const char* argument);
bool handleSubcommandInt(const char* groupName, const char* settingName, int* target, int min, int max, const char* subCommand, const char* argument);
bool handleSubcommandBool(const char* groupName, const char* settingName, bool* target, const char* subCommand, const char* argument);

#endif