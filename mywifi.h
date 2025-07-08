#ifndef WIFI_H_
#define WIFI_H_

#define WIFI_SSID_MAXLENGTH 32
#define WIFI_PASSWORD_MAXLENGTH 32

struct SettingsWifi {
  char ssid[WIFI_SSID_MAXLENGTH+1];
  char password[WIFI_PASSWORD_MAXLENGTH+1];
};

#define SETTING_WIFI            "wifi"
#define SETTING_WIFI_SSID       "ssid"
#define SETTING_WIFI_PASSWORD   "password"

void wifiInit();
void wifiSettingsFactory();
void wifiSettingsDump();
bool wifiReceiveCommand(const char* subCommand, const char* argument);

#endif