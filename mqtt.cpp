#include "mqtt.h"
#include "mqtt-ha.h"
#include "mqtt-strings.h"
#include "main.h"
#include "utils.h"
#include <Arduino.h>
#include "settings.h"
#include "display.h"
#include "device.h"
#include "brightness.h"
#include <WiFi.h>
#include "ESP32MQTTClient.h"

ESP32MQTTClient mqttClient;

/* Session variables */
int mqttBrightnessStore = -1;
int mqttIlluminanceStore = -1;
bool mqttPrepareReset = false;

/* Publishers */
void mqttPublishBrightnessChanged() {
  char brightnessStr[8];
  itoa(mqttBrightnessStore, brightnessStr, 10);
  mqttClient.publish(mqttBrightnessStateTopic, brightnessStr, 0, true);
}

void mqttPublishSwitchChanged() {
  mqttClient.publish(mqttSwitchStateTopic, RUNTIME_BRIGHTNESS.sw ? MQTT_ON : MQTT_OFF, 0, true);
}

void mqttPublishAutobrightnessChanged() {
  mqttClient.publish(mqttAutobrightnessStateTopic, RUNTIME_BRIGHTNESS.isAuto ? MQTT_ON : MQTT_OFF, 0, true);
}

void mqttPublishIlluminanceChanged() {
  char illuminanceStr[8];
  itoa(mqttIlluminanceStore, illuminanceStr, 10);
  mqttClient.publish(mqttIlluminanceStateTopic, illuminanceStr, 0, true);
}

/* Notifiers */
void mqttNotifyBrightnessChanged(int value) {
  mqttBrightnessStore = value;
  mqttPublishBrightnessChanged();
}

void mqttNotifyIlluminanceChanged(int value) {
  mqttIlluminanceStore = value;
  mqttPublishIlluminanceChanged();
}

/* Event handlers */
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

#define MAX_TOPIC_PIECES 4
#define MAX_TOPIC_PIECE_LENGTH 32

void mqttOnMessage(const std::string &topicSTR, const std::string &payloadSTR) {

  // Check base topic
  const char* topic = topicSTR.c_str();

  if ((strncmp(topic, mqttBaseTopic, mqttBaseTopicLength) != 0) || (topic[mqttBaseTopicLength] != '/'))
    return;

  // Tokenize
  bool topicPieceTooLong = false;
  bool tooManyTopicPieces = false;
  int topicPieceCount = 0;

  char topicPieces[MAX_TOPIC_PIECES][MAX_TOPIC_PIECE_LENGTH+1];
  int topicPieceIdx = 0;
  const char* topicPieceStart = topic+mqttBaseTopicLength+1;
  const char* topicPieceEnd = topicPieceStart;

  while ((topicPieceIdx < MAX_TOPIC_PIECES) && (*topicPieceEnd != '\0')) {
    topicPieceEnd = topicPieceStart;
    while ((*topicPieceEnd != '/') && (*topicPieceEnd != '\0'))
      topicPieceEnd++;
    int topicPieceLength = topicPieceEnd - topicPieceStart;
    if (topicPieceLength < MAX_TOPIC_PIECE_LENGTH) {
      memcpy(topicPieces[topicPieceIdx], topicPieceStart, topicPieceLength);
      topicPieces[topicPieceIdx][topicPieceLength] = '\0';
    } else {
      topicPieceTooLong = true;
    }
    topicPieceIdx++;
    topicPieceStart = topicPieceEnd+1;
  }

  if (*topicPieceEnd != '\0')
    tooManyTopicPieces = true;

  topicPieceCount = topicPieceIdx;

  if (topicPieceTooLong || tooManyTopicPieces || (topicPieceCount < 1))
    return;

  // Convert payload
  const char* payload = payloadSTR.c_str();

  if (strcmp(topicPieces[0], TOPIC_AUTOBRIGHTNESS_SET) == 0) {
    RUNTIME_BRIGHTNESS.isAuto = (strcmp(payload, MQTT_ON) == 0) & 1;
    mqttPublishAutobrightnessChanged();
    SM_RT_BRIGHTNESS.runtimeChanged();
  }

  if (strcmp(topicPieces[0], TOPIC_BRIGHTNESS_SET) == 0) {
    int brightnessValue = atoi(payload);
    RUNTIME_BRIGHTNESS.isAuto = 0;
    mqttPublishAutobrightnessChanged();
    RUNTIME_BRIGHTNESS.manual = brightnessValue;
    SM_RT_BRIGHTNESS.runtimeChanged();
  }

  if (strcmp(topicPieces[0], TOPIC_SWITCH_SET) == 0) {
    RUNTIME_BRIGHTNESS.sw = (strcmp(payload, MQTT_ON) == 0) & 1;
    mqttPublishSwitchChanged();
    SM_RT_BRIGHTNESS.runtimeChanged();
  }

  if (strcmp(topicPieces[0], TOPIC_RESET) == 0) {
    if (strcmp(payload, MQTT_RESET) == 0) {
      mqttPrepareReset = true;
    }
  }

  if ((strcmp(topicPieces[0], TOPIC_TEXT) == 0) && (topicPieceCount > 1)) {
    bool displayIdxOk;
    int displayIdx = satoi(topicPieces[1], &displayIdxOk);
    if (displayIdxOk && (displayIdx >= 0))
      mqttProcessMessageForDisplay(displayIdx, payload);
  }

}

void mqttOnMessageEmptyCallback(const std::string &topicSTR, const std::string &payload)
{ }

void onMqttConnect(esp_mqtt_client_handle_t client) // can't rename
{
  Serial.println(mqttBaseTopicWildcard);
  mqttClient.subscribe(mqttBaseTopicWildcard, mqttOnMessageEmptyCallback, 0);
  mqttHaAutoDisconvery();
  mqttClient.publish(mqttAvailabilityTopic, "online", 0, true);
  mqttPublishAutobrightnessChanged();
  mqttPublishBrightnessChanged();
  mqttPublishSwitchChanged();
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}

/* Init */
void mqttInit() {
  mqttStringsGenerate();
  mqttClient.setURL(SETTINGS_MQTT.broker, SETTINGS_MQTT.port, SETTINGS_MQTT.user, SETTINGS_MQTT.password);
  mqttClient.setMqttClientName(SETTINGS_DEVICE.name);
  mqttClient.setKeepAlive(30);
  mqttClient.setOnMessageCallback(mqttOnMessage);
  mqttClient.enableLastWillMessage(mqttAvailabilityTopic, "offline", true);
  mqttClient.loopStart();
}

/* Main loop */
void mqttMainLoop() {
  if (mqttPrepareReset)
    ESP.restart();
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