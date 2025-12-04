#ifndef OTA_H_
#define OTA_H_

#include "settings.h"

#define OTA_PASSWORD_MAXLENGTH 16

#define SETTING_OTA            "ota"
#define SETTING_OTA_ENABLE     "enable"
#define SETTING_OTA_PASSWORD   "password"

struct SettingsOta {
  bool enable;
  char password[OTA_PASSWORD_MAXLENGTH+1];
};

extern SettingsOta SETTINGS_OTA;
extern SettingsManager SM_OTA;

void otaInit();
void otaMainLoop();

#endif