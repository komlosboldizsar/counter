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
#include <FS.h>
#include <LittleFS.h>

ESP32MQTTClient mqttClient;

const char* OPTIONS_CONNTYPE[] = {"mqtt", "ws", NULL};

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

  if ((strcmp(topicPieces[0], TOPIC_TEXT) == 0) && (topicPieceCount > 1)) {
    bool displayIdxOk;
    int displayIdx = satoi(topicPieces[1], &displayIdxOk);
    if (displayIdxOk && (displayIdx >= 0))
      mqttProcessMessageForDisplay(displayIdx, payload);
    return;
  }

  if (strcmp(topicPieces[0], TOPIC_AUTOBRIGHTNESS_SET) == 0) {
    RUNTIME_BRIGHTNESS.isAuto = (strcmp(payload, MQTT_ON) == 0) & 1;
    mqttPublishAutobrightnessChanged();
    SM_RT_BRIGHTNESS.runtimeChanged();
    return;
  }

  if (strcmp(topicPieces[0], TOPIC_BRIGHTNESS_SET) == 0) {
    int brightnessValue = atoi(payload);
    RUNTIME_BRIGHTNESS.isAuto = 0;
    mqttPublishAutobrightnessChanged();
    RUNTIME_BRIGHTNESS.manual = brightnessValue;
    SM_RT_BRIGHTNESS.runtimeChanged();
    return;
  }

  if (strcmp(topicPieces[0], TOPIC_SWITCH_SET) == 0) {
    RUNTIME_BRIGHTNESS.sw = (strcmp(payload, MQTT_ON) == 0) & 1;
    mqttPublishSwitchChanged();
    SM_RT_BRIGHTNESS.runtimeChanged();
    return;
  }

  if (strcmp(topicPieces[0], TOPIC_RESET) == 0) {
    if (strcmp(payload, MQTT_RESET) == 0) {
      mqttPrepareReset = true;
    }
    return;
  }

}

void mqttOnMessageEmptyCallback(const std::string &topicSTR, const std::string &payload)
{ }

void onMqttConnect(esp_mqtt_client_handle_t client) // can't rename
{
  mqttClient.subscribe(mqttBaseTopicWildcard, mqttOnMessageEmptyCallback, 0);
  mqttHaAutoDiscoveryStart();
  mqttClient.publish(mqttAvailabilityTopic, MQTT_ONLINE, 0, true);
  mqttPublishAutobrightnessChanged();
  mqttPublishBrightnessChanged();
  mqttPublishSwitchChanged();
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
  mqttClient.onEventCallback(event);
}

/* Certs */
#define MQTT_CERT_MAXSIZE 2048
char mqttCaCert[MQTT_CERT_MAXSIZE+1], mqttClientCert[MQTT_CERT_MAXSIZE+1], mqttKey[MQTT_CERT_MAXSIZE+1];
const char* MQTT_CERT_PATH = "/certs";

bool mqttLoadCert(char* target, const char* name) {
  char filePath[128];
  strcpy(filePath, MQTT_CERT_PATH);
  strcat(filePath, "/");
  strcat(filePath, name);
  File file = LittleFS.open(filePath, "r");
  if(!file || file.isDirectory()){
    Serial.print("Failed to open MQTT cert file: [");
    Serial.print(filePath);
    Serial.println("]");
    return false;
  }
  int fileSize = file.size();
  if (fileSize > MQTT_CERT_MAXSIZE) {
    Serial.print("MQTT cert file too big: [");
    Serial.print(filePath);
    Serial.println("]");
    return false;
  }
  file.readBytes(target, fileSize);
  target[fileSize] = '\0';
  return true;
}

bool mqttLoadCerts() {
  LittleFS.begin(true);
  if (!mqttLoadCert(mqttCaCert, "ca"))
    return false;
  if (!mqttLoadCert(mqttClientCert, "client"))
    return false;
  if (!mqttLoadCert(mqttKey, "key"))
    return false;
  return true; 
}

/* Init */
void mqttInit() {

  mqttStringsGenerate();

  char mqttURI[8+MQTT_BROKER_MAXLENGTH+1+5+1]; // 8: "mqtts://", 1: ":", 5: "65535", 1: "\0"
  strcpy(mqttURI, SETTINGS_MQTT.conntype == MQTTCT_WS ? OPTIONS_CONNTYPE[1] : OPTIONS_CONNTYPE[0]);
  if (SETTINGS_MQTT.secure)
    strcat(mqttURI, "s");
  strcat(mqttURI, "://");
  strcat(mqttURI, SETTINGS_MQTT.broker);
  strcat(mqttURI, ":");
  char mqttPort[5+1];
  itoa(SETTINGS_MQTT.port, mqttPort, 10);
  strcat(mqttURI, mqttPort);


  mqttClient.setURI(mqttURI, SETTINGS_MQTT.user, SETTINGS_MQTT.password);
  mqttClient.setMqttClientName(SETTINGS_DEVICE.name);
  mqttClient.setKeepAlive(30);
  mqttClient.setOnMessageCallback(mqttOnMessage);
  mqttClient.enableLastWillMessage(mqttAvailabilityTopic, MQTT_OFFLINE, true);

  if (SETTINGS_MQTT.secure) {
    mqttLoadCerts();
    mqttClient.setCaCert(mqttCaCert);
    mqttClient.setClientCert(mqttClientCert);
    mqttClient.setKey(mqttKey);
  }

  mqttClient.loopStart();

}

/* Main loop */
void mqttMainLoop() {
  if (mqttPrepareReset)
    ESP.restart();
}

/* Settings */
SettingsMqtt SETTINGS_MQTT;

SettingValueManagerEnum SVM_MQTT_CONNTYPE(SETTING_MQTT_CONNTYPE, &SETTINGS_MQTT.conntype, (int)MQTTCT_MQTT, OPTIONS_CONNTYPE);
SettingValueManagerBool SVM_MQTT_SECURE(SETTING_MQTT_SECURE, &SETTINGS_MQTT.secure, false);
SettingValueManagerString SVM_MQTT_BROKER(SETTING_MQTT_BROKER, SETTINGS_MQTT.broker, "192.168.1.1", -1, MQTT_BROKER_MAXLENGTH);
SettingValueManagerInt SVM_MQTT_PORT(SETTING_MQTT_PORT, &SETTINGS_MQTT.port, 1883, 1, 65535);
SettingValueManagerString SVM_MQTT_USER(SETTING_MQTT_USER, SETTINGS_MQTT.user, "counter", -1, MQTT_USER_MAXLENGTH);
SettingValueManagerString SVM_MQTT_PASSWORD(SETTING_MQTT_PASSWORD, SETTINGS_MQTT.password, "counter", -1, MQTT_PASSWORD_MAXLENGTH);
SettingValueManagerString SVM_MQTT_TOPIC(SETTING_MQTT_TOPIC, SETTINGS_MQTT.topic, "counter", -1, MQTT_TOPIC_MAXLENGTH);

SettingValueManager* SM_MQTT_MEMBERS[] = { &SVM_MQTT_CONNTYPE, &SVM_MQTT_SECURE, &SVM_MQTT_BROKER, &SVM_MQTT_PORT, &SVM_MQTT_USER, &SVM_MQTT_PASSWORD, &SVM_MQTT_TOPIC, NULL };
SettingsManager SM_MQTT(SETTING_MQTT, LONGTIME, SETTINGS_DATA(SETTINGS_MQTT), SM_MQTT_MEMBERS);