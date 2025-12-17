#include "mqtt-strings.h"
#include "esp_mac.h"
#include <Arduino.h>

const char TOPIC_BASE[] = "counter";
const char TOPIC_AVAILABILITY[] = "availability";
const char TOPIC_AUTOBRIGHTNESS_SET[] = "autobrightnessSet";
const char TOPIC_AUTOBRIGHTNESS_STATE[] = "autobrightnessState";
const char TOPIC_BRIGHTNESS_SET[] = "brightnessSet";
const char TOPIC_BRIGHTNESS_STATE[] = "brightnessState";
const char TOPIC_SWITCH_SET[] = "switchSet";
const char TOPIC_SWITCH_STATE[] = "switchState";
const char TOPIC_ILLUMINANCE_STATE[] = "illuminanceState";
const char TOPIC_RESET[] = "reset";
const char TOPIC_TEXT[] = "text";

const char MQTT_ON[] = "ON";
const char MQTT_OFF[] = "OFF";
const char MQTT_ONLINE[] = "online";
const char MQTT_OFFLINE[] = "offline";
const char MQTT_RESET[] = "reset";

const char HEX_CHRS[] = "0123456789ABCDEF";

char mqttMacClean[13];

int mqttBaseTopicLength;

char mqttBaseTopic[sizeof(TOPIC_BASE)+sizeof(mqttMacClean)];
char mqttBaseTopicWildcard[sizeof(mqttBaseTopic)+2];
char mqttAvailabilityTopic[sizeof(mqttBaseTopic)+sizeof(TOPIC_AVAILABILITY)];
char mqttAutobrightnessStateTopic[sizeof(mqttBaseTopic)+sizeof(TOPIC_AUTOBRIGHTNESS_STATE)];
char mqttBrightnessStateTopic[sizeof(mqttBaseTopic)+sizeof(TOPIC_BRIGHTNESS_STATE)];
char mqttSwitchStateTopic[sizeof(mqttBaseTopic)+sizeof(TOPIC_SWITCH_STATE)];
char mqttIlluminanceStateTopic[sizeof(mqttBaseTopic)+sizeof(TOPIC_ILLUMINANCE_STATE)];

void mqttMacGenerate() {
  uint8_t macHex[6];
  esp_efuse_mac_get_default(macHex);
  int iHex = 0, iClean = 0;
  for (int i = 0; i < 6; i++) {
    mqttMacClean[iClean++] = HEX_CHRS[(macHex[iHex] & 0xF0) >> 4];
    mqttMacClean[iClean++] = HEX_CHRS[macHex[iHex] & 0x0F];
    iHex++;
  }
  mqttMacClean[12] = '\0';
}

void mqttStringsGenerate() {

  strcpy(mqttBaseTopic, TOPIC_BASE);
  strcat(mqttBaseTopic, "/");
  strcat(mqttBaseTopic, mqttMacClean);
  mqttBaseTopicLength = strlen(mqttBaseTopic);
  
  strcpy(mqttBaseTopicWildcard, mqttBaseTopic);
  strcat(mqttBaseTopicWildcard, "/#");

  strcpy(mqttAvailabilityTopic, mqttBaseTopic);
  strcat(mqttAvailabilityTopic, "/");
  strcat(mqttAvailabilityTopic, TOPIC_AVAILABILITY);

  strcpy(mqttAutobrightnessStateTopic, mqttBaseTopic);
  strcat(mqttAutobrightnessStateTopic, "/");
  strcat(mqttAutobrightnessStateTopic, TOPIC_AUTOBRIGHTNESS_STATE);

  strcpy(mqttBrightnessStateTopic, mqttBaseTopic);
  strcat(mqttBrightnessStateTopic, "/");
  strcat(mqttBrightnessStateTopic, TOPIC_BRIGHTNESS_STATE);

  strcpy(mqttSwitchStateTopic, mqttBaseTopic);
  strcat(mqttSwitchStateTopic, "/");
  strcat(mqttSwitchStateTopic, TOPIC_SWITCH_STATE);

  strcpy(mqttIlluminanceStateTopic, mqttBaseTopic);
  strcat(mqttIlluminanceStateTopic, "/");
  strcat(mqttIlluminanceStateTopic, TOPIC_ILLUMINANCE_STATE);
  
}