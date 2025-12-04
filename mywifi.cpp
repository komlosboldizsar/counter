#include "mywifi.h"
#include "main.h"
#include <Arduino.h>
#include <WiFi.h>
#include "settings.h"
#include "device.h"

void wifiInit() {
  WiFi.setHostname(SETTINGS_DEVICE.name);
  WiFi.setAutoReconnect(true);
  WiFi.begin(SETTINGS_WIFI.ssid, SETTINGS_WIFI.password);
}

/* Settings */
SettingsWifi SETTINGS_WIFI;

SettingValueManagerString SVM_WIFI_SSID(SETTING_WIFI_SSID, SETTINGS_WIFI.ssid, "COUNTER", -1, WIFI_SSID_MAXLENGTH);
SettingValueManagerString SVM_WIFI_PASSWORD(SETTING_WIFI_PASSWORD, SETTINGS_WIFI.password, "counter", -1, WIFI_PASSWORD_MAXLENGTH);

SettingValueManager* SM_WIFI_MEMBERS[] = { &SVM_WIFI_SSID, &SVM_WIFI_PASSWORD, NULL };
SettingsManager SM_WIFI(SETTING_WIFI, LONGTIME, SETTINGS_DATA(SETTINGS_WIFI), SM_WIFI_MEMBERS);
