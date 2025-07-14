#include "mqtt.h"
#include "main.h"
#include <Arduino.h>
#include "settings.h"
#include "display.h"
#include <WiFi.h>
#include <PubSubClient.h>

#define DEBUG_MQTT

static WiFiClient mqttWifiClient;
static PubSubClient mqttClient;

void mqttCallback(char* topic, byte* message, unsigned int length);

void mqttInit() {
  mqttClient = PubSubClient(SETTINGS.MQTT.broker, 1883, mqttWifiClient);
  mqttClient.setCallback(mqttCallback);
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

void mqttCallback(char* topic, byte* message, unsigned int length) {

  #ifdef DEBUG_MQTT
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  #endif
  
  char strBuffer[129];
  strncpy(strBuffer, (char*)message, 128);
  strBuffer[length > 128 ? 128 : length] = '\0';
  Serial.println(strBuffer);

  int basetopicLength = strlen(SETTINGS.MQTT.topic);
  int topicLength = strlen(topic);

  if (strncmp(topic, SETTINGS.MQTT.topic, basetopicLength) == 0) {

    if ((topicLength >= basetopicLength + 2) && (topic[basetopicLength] == '/')) {
      char* strPtr = topic+basetopicLength+1;
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
      Serial.print("Message for display: ");
      Serial.println(idx, DEC);
      mqttProcessMessageForDisplay(idx, strBuffer);
      #endif

    }

  }
 
}

#define MQTT_RECONNECT_INTERVAL 5000
unsigned long mqttLastReconnectTry = 0;

void mqttConnect(unsigned long now) {

  if (mqttClient.connected())
    return;
  if ((mqttLastReconnectTry != 0) && (mqttLastReconnectTry < now) && (now < mqttLastReconnectTry + MQTT_RECONNECT_INTERVAL))
    return;
  // IMPROTANT _NOT_ TO CHECK WIFI STATUS!!
  /*if (WiFi.status() != WL_CONNECTED)
    return;*/
  
  #ifdef DEBUG_MQTT
  Serial.println("MQTT connecting...");
  #endif
  if (mqttClient.connect(SETTINGS.DEVICE.name, SETTINGS.MQTT.user, SETTINGS.MQTT.password)) {
    #ifdef DEBUG_MQTT
    Serial.println("MQTT connected.");
    #endif
    char topicBuffer[MQTT_TOPIC_MAXLENGTH+32+1];
    strcpy(topicBuffer, SETTINGS.MQTT.topic);
    strcat(topicBuffer, "/+");
    mqttClient.subscribe(topicBuffer);
    strcpy(topicBuffer, SETTINGS.MQTT.topic);
    strcat(topicBuffer, "/hello");
    mqttClient.publish(topicBuffer, "hello");
  } else {
    #ifdef DEBUG_MQTT
    Serial.print("MQTT connection failed, code: [");
    Serial.print(mqttClient.state());
    Serial.println("]");
    #endif
  }

  mqttLastReconnectTry = now;

}

void mqttMainLoop(unsigned long now) {
  mqttConnect(now);
  mqttClient.loop();
}