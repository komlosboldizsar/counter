#ifndef DEVICE_H_
#define DEVICE_H_

#define DEVICE_NAME_MAXLENGTH 8

struct SettingsDevice {
  char name[DEVICE_NAME_MAXLENGTH+1];
  bool showmac;
};

#define SETTING_DEVICE            "device"
#define SETTING_DEVICE_NAME       "name"
#define SETTING_DEVICE_SHOWMAC    "showmac"

void deviceSettingsFactory();
void deviceSettingsDump();
bool deviceReceiveCommand(const char* subCommand, const char* argument);

#endif