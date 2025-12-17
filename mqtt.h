#ifndef MQTT_H_
#define MQTT_H_

#include "settings.h"

#define MQTT_BROKER_MAXLENGTH    64
#define MQTT_USER_MAXLENGTH      32
#define MQTT_PASSWORD_MAXLENGTH  32
#define MQTT_TOPIC_MAXLENGTH     32

#define SETTING_MQTT            "mqtt"
#define SETTING_MQTT_CONNTYPE   "conntype"
#define SETTING_MQTT_SECURE     "secure"
#define SETTING_MQTT_BROKER     "broker"
#define SETTING_MQTT_PORT       "port"
#define SETTING_MQTT_USER       "user"
#define SETTING_MQTT_PASSWORD   "password"
#define SETTING_MQTT_TOPIC      "topic"

enum MqttConnType {
  MQTTCT_MQTT,
  MQTTCT_WS
};

struct SettingsMqtt {
public:
  int conntype;
  bool secure;
  char broker[MQTT_BROKER_MAXLENGTH+1];
  int port;
  char user[MQTT_USER_MAXLENGTH+1];
  char password[MQTT_PASSWORD_MAXLENGTH+1];
  char topic[MQTT_TOPIC_MAXLENGTH+1];
};

extern SettingsMqtt SETTINGS_MQTT;
extern SettingsManager SM_MQTT;

void mqttInit();
void mqttMainLoop();

void mqttNotifyBrightnessChanged(int value);
void mqttNotifyIlluminanceChanged(int value);

#endif