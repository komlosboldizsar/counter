#ifndef DEVICE_H_
#define DEVICE_H_

#include "settings.h"

#define DEVICE_NAME_MAXLENGTH 8

#define SETTING_DEVICE            "device"
#define SETTING_DEVICE_NAME       "name"
#define SETTING_DEVICE_SHOWMAC    "showmac"

struct SettingsDevice {
public:
  char name[DEVICE_NAME_MAXLENGTH+1];
  bool showmac;
};

extern SettingsDevice SETTINGS_DEVICE;
extern SettingsManager SM_DEVICE;

#endif