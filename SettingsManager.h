#ifndef SETTINGSMANAGER_H_
#define SETTINGSMANAGER_H_

#include <cstddef>
#include "SettingValueManager.h"

enum SettingType {
  LONGTIME,
  RUNTIME
};

class SettingsManager {
public:
  const char* _name;
  SettingType _type;
protected:
  void* _data;
  int _dataSize;
  SettingValueManager** _members;
  int32_t _runtimeLastChangeSince = -1;
public:
  SettingsManager(const char* groupName, SettingType type, void* data, int dataSize, SettingValueManager** members);
  static void init();
  void load();
  void save();
  void factoryValues();
  void dumpValues(void(*outputFunc)(const char* message));
  void exportValues(void(*outputFunc)(const char* message));
  bool receiveCommand(const char* subCommand, const char* argument, void(*outputFunc)(const char* message));
  void runtimeChanged();
  void runtimeMainLoop();
};

#define SETTINGS_DATA(VAR)          &VAR, sizeof(VAR)

#endif