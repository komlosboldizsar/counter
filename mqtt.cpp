#include "mqtt.h"
#include "main.h"
#include <Arduino.h>
#include "settings.h"
#include "display.h"
#include "device.h"
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

  int basetopicLength = strlen(SETTINGS_MQTT.topic);
  int topicLength = strlen(topic);

  if (strncmp(topic, SETTINGS_MQTT.topic, basetopicLength) == 0) {

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
    char topic[sizeof(SETTINGS_MQTT.topic)+16];
    strcpy(topic, SETTINGS_MQTT.topic);
    strcat(topic, "/#");
    mqttClient.subscribe(topic, mqttOnMessageEmptyCallback);
    strcpy(topic, SETTINGS_MQTT.topic);
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
  mqttClient.setURL(SETTINGS_MQTT.broker, SETTINGS_MQTT.port, SETTINGS_MQTT.user, SETTINGS_MQTT.password);
  mqttClient.setMqttClientName(SETTINGS_DEVICE.name);
  mqttClient.setKeepAlive(30);
  mqttClient.setOnMessageCallback(mqttOnMessage);
  mqttClient.loopStart();
}

/* Settings */
SettingsMqtt SETTINGS_MQTT;

SettingValueManagerString SVM_MQTT_BROKER(SETTING_MQTT_BROKER, SETTINGS_MQTT.broker, "192.168.1.1", -1, MQTT_BROKER_MAXLENGTH);
SettingValueManagerInt SVM_MQTT_PORT(SETTING_MQTT_PORT, &SETTINGS_MQTT.port, 1883, 1, 65535);
SettingValueManagerString SVM_MQTT_USER(SETTING_MQTT_USER, SETTINGS_MQTT.user, "counter", -1, MQTT_USER_MAXLENGTH);
SettingValueManagerString SVM_MQTT_PASSWORD(SETTING_MQTT_PASSWORD, SETTINGS_MQTT.password, "counter", -1, MQTT_PASSWORD_MAXLENGTH);
SettingValueManagerString SVM_MQTT_TOPIC(SETTING_MQTT_TOPIC, SETTINGS_MQTT.topic, "counter", -1, MQTT_TOPIC_MAXLENGTH);

SettingValueManager* SM_MQTT_MEMBERS[] = { &SVM_MQTT_BROKER, &SVM_MQTT_PORT, &SVM_MQTT_USER, &SVM_MQTT_PASSWORD, &SVM_MQTT_TOPIC, NULL };
SettingsManager SM_MQTT(SETTING_MQTT, LONGTIME, SETTINGS_DATA(SETTINGS_MQTT), SM_MQTT_MEMBERS);