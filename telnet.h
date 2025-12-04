#ifndef TELNET_H_
#define TELNET_H_

#include "settings.h"

#define SETTING_TELNET            "telnet"
#define SETTING_TELNET_ENABLE     "enable"
#define SETTING_TELNET_PORT       "port"

struct SettingsTelnet {
  bool enable;
  int port;
};

extern SettingsTelnet SETTINGS_TELNET;
extern SettingsManager SM_TELNET;

void telnetInit();
void telnetMainLoop();

void telnetWrite(const char* message);

#endif