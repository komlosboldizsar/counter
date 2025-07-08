#include "mywifi.h"
#include "main.h"
#include <Arduino.h>
#include <WiFi.h>
#include "settings.h"

void wifiInit() {
  WiFi.setHostname(SETTINGS.DEVICE.name);
  WiFi.setAutoReconnect(true);
  WiFi.begin(SETTINGS.WIFI.ssid, SETTINGS.WIFI.password);
}

void wifiSettingsFactory() {
  strcpy(SETTINGS.WIFI.ssid, "CLOCK");
  strcpy(SETTINGS.WIFI.password, "clock");
}

void wifiSettingsDump() {
  settingsDumpPartStart(SETTING_WIFI);
  settingsDumpValueString(SETTING_WIFI_SSID, SETTINGS.WIFI.ssid);
  settingsDumpValueString(SETTING_WIFI_PASSWORD, SETTINGS.WIFI.password);
}

bool wifiReceiveCommand(const char* subCommand, const char* argument) {

  bool handled = false;
  
  handled = handleSubcommandString(SETTING_WIFI, SETTING_WIFI_SSID, SETTINGS.WIFI.ssid, -1, WIFI_SSID_MAXLENGTH, subCommand, argument);
  if (handled)
    return true;

  handleSubcommandString(SETTING_WIFI, SETTING_WIFI_PASSWORD, SETTINGS.WIFI.password, -1, WIFI_PASSWORD_MAXLENGTH, subCommand, argument);
  if (handled)
    return true;

  return false;
  
}