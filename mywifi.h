#ifndef WIFI_H_
#define WIFI_H_

#include "settings.h"

#define WIFI_SSID_MAXLENGTH      32
#define WIFI_PASSWORD_MAXLENGTH  32

#define SETTING_WIFI             "wifi"
#define SETTING_WIFI_SSID        "ssid"
#define SETTING_WIFI_PASSWORD    "password"

struct SettingsWifi {
  char ssid[WIFI_SSID_MAXLENGTH+1];
  char password[WIFI_PASSWORD_MAXLENGTH+1];
};

extern SettingsWifi SETTINGS_WIFI;
extern SettingsManager SM_WIFI;

void wifiInit();

#endif