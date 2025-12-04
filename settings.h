#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <Arduino.h>
#include "SettingValueManager.h"
#include "SettingValueManagerString.h"
#include "SettingValueManagerInt.h"
#include "SettingValueManagerBool.h"
#include "SettingValueManagerEnum.h"
#include "SettingsManager.h"

void settingsInit();
void settingsMainLoop();

void settingsSave(void(*outputFunc)(const char* message) = NULL);
void settingsLoad();
void settingsFactory(const char* categoryName, void(*outputFunc)(const char* message) = NULL);
void settingsDump(const char* categoryName, void(*outputFunc)(const char* message));
void settingsExport(const char* categoryName, void(*outputFunc)(const char* message));

void settingsReceiveCommand(const char* mainCommand, const char* subCommand, const char* argument, void(*outputFunc)(const char* message));
void settingsReceiveLine(const char* line, void(*outputFunc)(const char* message));

#endif