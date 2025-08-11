#include "mqtt.h"
#include "main.h"
#include <Arduino.h>
#include "settings.h"
#include "display.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"

#define DEBUG_MQTT

ESP32MQTTClient mqttClient;

void mqttProcessMessageForDisplay(int idx, const char* message) {

  bool valid = true;
  const char* mPtr = message;
  int numbers = 0;
  int decimals = 0;
  int firstDecimal = -1;

  while (*mPtr != '\0') {
    if ((*mPtr >= '0') && (*mPtr <= '9')) {
      numbers++;
    } else if (*mPtr == '.') {
      decimals++;
      if (decimals == 1)
        firstDecimal = mPtr - message;
    } else {
      valid = false;
    }
    mPtr++;
  }

  if ((decimals > 1) || (numbers > 4) || (firstDecimal == 0))
    valid = false;

  if (!valid)
    return;

  displayClear(idx);

  mPtr = message;
  int usefulLength = numbers + decimals;
  int c = 0;
  while (c < numbers) {
    bool dp = (c+1 == firstDecimal);
    displayDigit(idx, 4 - numbers + c, *mPtr - '0', dp);
    c++;
    mPtr++;
    if (dp)
      mPtr++;
  }

}

void mqttOnMessage(const std::string &topicSTR, const std::string &payload) {

  const char* topic = topicSTR.c_str();
  const char* strBuffer = payload.c_str();

  int basetopicLength = strlen(SETTINGS.MQTT.topic);
  int topicLength = strlen(topic);

  if (strncmp(topic, SETTINGS.MQTT.topic, basetopicLength) == 0) {

    if ((topicLength >= basetopicLength + 2) && (topic[basetopicLength] == '/')) {
      const char* strPtr = topic+basetopicLength+1;
      bool ok = true;
      int idx = 0;
      while (*strPtr != '\0') {
        if ((*strPtr >= '0') && (*strPtr <= '9')) {
          idx *= 10;
          idx += *strPtr - '0';
        } else {
          ok = false;
        }
        strPtr++;
      }

      #ifdef DEBUG_MQTT
      Serial.print("Message for display #");
      Serial.print(idx, DEC);
      Serial.print(": [");
      Serial.print(strBuffer);
      Serial.println("]");
      mqttProcessMessageForDisplay(idx, strBuffer);
      #endif

    }

  }

}

void mqttOnMessageEmptyCallback(const std::string &topicSTR, const std::string &payload)
{ }

void onMqttConnect(esp_mqtt_client_handle_t client)
{
  if (mqttClient.isMyTurn(client)) {
    char topic[sizeof(SETTINGS.MQTT.topic)+16];
    strcpy(topic, SETTINGS.MQTT.topic);
    strcat(topic, "/#");
    mqttClient.subscribe(topic, mqttOnMessageEmptyCallback);
    strcpy(topic, SETTINGS.MQTT.topic);
    strcat(topic, "/hello");
    mqttClient.publish(topic, "hello");
  }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
    mqttClient.onEventCallback(event);
}

void mqttInit() {
  mqttClient.setURL(SETTINGS.MQTT.broker, SETTINGS.MQTT.port, SETTINGS.MQTT.user, SETTINGS.MQTT.password);
  mqttClient.setMqttClientName(SETTINGS.DEVICE.name);
  mqttClient.setKeepAlive(30);
  mqttClient.setOnMessageCallback(mqttOnMessage);
  mqttClient.loopStart();
}

void mqttSettingsFactory() {
  strcpy(SETTINGS.MQTT.broker, "192.168.1.1");
  SETTINGS.MQTT.port = 1883;
  strcpy(SETTINGS.MQTT.user, "");
  strcpy(SETTINGS.MQTT.password, "");
  strcpy(SETTINGS.MQTT.topic, "counter");
}

void mqttSettingsDump() {
  settingsDumpPartStart(SETTING_MQTT);
  settingsDumpValueString(SETTING_MQTT_BROKER, SETTINGS.MQTT.broker);
  settingsDumpValueInt(SETTING_MQTT_PORT, SETTINGS.MQTT.port);
  settingsDumpValueString(SETTING_MQTT_USER, SETTINGS.MQTT.user);
  settingsDumpValueString(SETTING_MQTT_PASSWORD, SETTINGS.MQTT.password);
  settingsDumpValueString(SETTING_MQTT_TOPIC, SETTINGS.MQTT.topic);
}

bool mqttReceiveCommand(const char* subCommand, const char* argument) {

  bool handled = false;
  
  handled = handleSubcommandString(SETTING_MQTT, SETTING_MQTT_BROKER, SETTINGS.MQTT.broker, -1, MQTT_BROKER_MAXLENGTH, subCommand, argument);
  if (handled)
    return true;

  handled = handleSubcommandInt(SETTING_MQTT, SETTING_MQTT_PORT, &SETTINGS.MQTT.port, 1, 65535, subCommand, argument);
  if (handled)
    return true;

  handleSubcommandString(SETTING_MQTT, SETTING_MQTT_USER, SETTINGS.MQTT.user, -1, MQTT_USER_MAXLENGTH, subCommand, argument);
  if (handled)
    return true;

  handleSubcommandString(SETTING_MQTT, SETTING_MQTT_PASSWORD, SETTINGS.MQTT.password, -1, MQTT_PASSWORD_MAXLENGTH, subCommand, argument);
  if (handled)
    return true;

  handleSubcommandString(SETTING_MQTT, SETTING_MQTT_TOPIC, SETTINGS.MQTT.topic, -1, MQTT_TOPIC_MAXLENGTH, subCommand, argument);
  if (handled)
    return true;

  return false;
  
}