#ifndef MQTT_H_
#define MQTT_H_

#define MQTT_BROKER_MAXLENGTH    64
#define MQTT_USER_MAXLENGTH      32
#define MQTT_PASSWORD_MAXLENGTH  32
#define MQTT_TOPIC_MAXLENGTH     16

struct SettingsMqtt {
  char broker[MQTT_BROKER_MAXLENGTH+1];
  int port;
  char user[MQTT_USER_MAXLENGTH+1];
  char password[MQTT_PASSWORD_MAXLENGTH+1];
  char topic[MQTT_TOPIC_MAXLENGTH+1];
};

#define SETTING_MQTT            "mqtt"
#define SETTING_MQTT_BROKER     "broker"
#define SETTING_MQTT_PORT       "port"
#define SETTING_MQTT_USER       "user"
#define SETTING_MQTT_PASSWORD   "password"
#define SETTING_MQTT_TOPIC      "topic"

void mqttInit();
void mqttSettingsFactory();
void mqttSettingsDump();
void mqttMainLoop(unsigned long now);
bool mqttReceiveCommand(const char* subCommand, const char* argument);

#endif