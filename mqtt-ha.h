#ifndef MQTT_HA_H
#define MQTT_HA_H

#include "settings.h"

#define MQTT_HA_DISCOVERYTOPIC_MAXLENGTH       32
#define MQTT_HA_FRIENDLYNAME_MAXLENGTH         32

#define SETTING_MQTT_HA                        "mqttha"
#define SETTING_MQTT_HA_ENABLE                 "enable"
#define SETTING_MQTT_HA_DISCOVERYTOPIC         "discoverytopic"
#define SETTING_MQTT_HA_FRIENDLYNAME           "friendlyname"

struct SettingsMqttHa {
  bool enable;
  char discoverytopic[MQTT_HA_DISCOVERYTOPIC_MAXLENGTH+1];
  char friendlyname[MQTT_HA_FRIENDLYNAME_MAXLENGTH+1];
};

extern SettingsMqttHa SETTINGS_MQTT_HA;
extern SettingsManager SM_MQTT_HA;

void mqttHaAutoDiscoveryStart();

#endif