#include "telnet.h"
#include "utils.h"
#include "main.h"
#include "settings.h"
#include "device.h"
#include <Arduino.h>
#include "ESPTelnet.h"

ESPTelnet telnet;

void telnetWrite(const char* message) {
  telnet.print(message);
}

void telnetOnInput(String str) {
  settingsReceiveLine(str.c_str(), telnetWrite);
}

void telnetOnConnect(String ip) {
  deviceIntroduce(telnetWrite);
}

void telnetInit() {
  if (!SETTINGS_TELNET.enable)
    return;
  telnet.onInputReceived(telnetOnInput);
  telnet.onConnect(telnetOnConnect);
  telnet.begin(SETTINGS_TELNET.port, false);
}

void telnetMainLoop() {
  if (!SETTINGS_TELNET.enable)
    return;
  telnet.loop();
}

/* Settings */
SettingsTelnet SETTINGS_TELNET;

SettingValueManagerBool SVM_TELNET_ENABLE(SETTING_TELNET_ENABLE, &SETTINGS_TELNET.enable, true);
SettingValueManagerInt SVM_TELNET_PORT(SETTING_TELNET_PORT, &SETTINGS_TELNET.port, 23, 1, 65535);

SettingValueManager* SM_TELNET_MEMBERS[] = { &SVM_TELNET_ENABLE, &SVM_TELNET_PORT, NULL };
SettingsManager SM_TELNET(SETTING_TELNET, LONGTIME, SETTINGS_DATA(SETTINGS_TELNET), SM_TELNET_MEMBERS);
