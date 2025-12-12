#include "mqtt-ha.h"
#include "mqtt-strings.h"
#include "mqtt.h"
#include "utils.h"
#include "json.h"
#include <Arduino.h>
#include "ESP32MQTTClient.h"

extern ESP32MQTTClient mqttClient;

#define CONCAT_TOPIC(...)           CONCAT(topic, 128, __VA_ARGS__)
#define PUBLISH()                   mqttClient.publish(topic, buffer, 0, true);

void mqttHaAutoDiscoveryLight() {

  JSON_BEGIN();
    // Device
    JSON_BEGIN_SECTION("dev");
      JSON_VALUE("identifiers", "[\"", "counter_", mqttMacClean, "\"]");
      JSON_PROPERTY("manufacturer", "komlosboldizsar");
      JSON_PROPERTY("model", "4N-digit counter");
      JSON_PROPERTY("model_id", "COUNTER");
      JSON_PROPERTY("name", SETTINGS_MQTT_HA.friendlyname);
    JSON_END_SECTION();
    // Identifiers
    JSON_PROPERTY("name", "Light");
    JSON_PROPERTY("object_id", SETTINGS_MQTT_HA.friendlyname, "_light");
    JSON_PROPERTY("uniq_id", "counter_", mqttMacClean);
    // Topics
    JSON_PROPERTY("avty_t", mqttAvailabilityTopic);
    JSON_PROPERTY("bri_cmd_t", "counter/", mqttMacClean, "/brightnessSet");
    JSON_PROPERTY("bri_stat_t", mqttBrightnessStateTopic);
    JSON_PROPERTY("cmd_t", "counter/", mqttMacClean, "/switchSet");
    JSON_PROPERTY("stat_t", mqttSwitchStateTopic);
    // Entity-related data
    JSON_PROPERTY("bri_scl", "15");
    JSON_VALUE("sup_clrm", "[\"brightness\"]");
  JSON_END();
  
  CONCAT_TOPIC(SETTINGS_MQTT_HA.discoverytopic, "/light/counter_", mqttMacClean, "/light/config");
  PUBLISH();
  
}

void mqttHaAutoDiscoveryAutobrightness() {

  JSON_BEGIN();
    // Device
    JSON_BEGIN_SECTION("dev");
      JSON_VALUE("identifiers", "[\"", "counter_", mqttMacClean, "\"]");
    JSON_END_SECTION();
    // Identifiers
    JSON_PROPERTY("name", "Auto brightness");
    JSON_PROPERTY("object_id", SETTINGS_MQTT_HA.friendlyname, "_autobrigthness");
    JSON_PROPERTY("uniq_id", "counter_", mqttMacClean, "_autobrightness");
    // Topics
    JSON_PROPERTY("avty_t", mqttAvailabilityTopic);
    JSON_PROPERTY("cmd_t", "counter/", mqttMacClean, "/autobrightnessSet");
    JSON_PROPERTY("stat_t", mqttAutobrightnessStateTopic);
    // Entity-related data
    JSON_PROPERTY("icon", "mdi:brightness-auto");
  JSON_END();
  
  CONCAT_TOPIC(SETTINGS_MQTT_HA.discoverytopic, "/switch/counter_", mqttMacClean, "_autobrightness/switch/config");
  PUBLISH();

}

void mqttHaAutoDiscoveryIlluminance() {

  JSON_BEGIN();
    // Device
    JSON_BEGIN_SECTION("dev");
      JSON_VALUE("identifiers", "[\"", "counter_", mqttMacClean, "\"]");
    JSON_END_SECTION();
    // Identifiers
    JSON_PROPERTY("name", "Light sensor");
    JSON_PROPERTY("object_id", SETTINGS_MQTT_HA.friendlyname, "_illuminance");
    JSON_PROPERTY("uniq_id", "counter_", mqttMacClean, "_illuminance");
    // Topics
    JSON_PROPERTY("avty_t", mqttAvailabilityTopic);
    JSON_PROPERTY("stat_t", mqttIlluminanceStateTopic);
    // Entity-related data
    JSON_PROPERTY("dev_cla", "illuminance");
    JSON_PROPERTY("exp_aft", "300");
    JSON_PROPERTY("frc_upd", "0");
    JSON_PROPERTY("stat_cla", "measurement");
    JSON_PROPERTY("sug_dsp_prc", "0");
  JSON_END();
  
  CONCAT_TOPIC(SETTINGS_MQTT_HA.discoverytopic, "/sensor/counter_", mqttMacClean, "_illuminance/sensor/config");
  PUBLISH();

}

void mqttHaAutoDiscoveryReset() {

  JSON_BEGIN();
    // Device
    JSON_BEGIN_SECTION("dev");
      JSON_VALUE("identifiers", "[\"", "counter_", mqttMacClean, "\"]");
    JSON_END_SECTION();
    // Identifiers
    JSON_PROPERTY("name", "Reset");
    JSON_PROPERTY("object_id", SETTINGS_MQTT_HA.friendlyname, "_reset");
    JSON_PROPERTY("uniq_id", "counter_", mqttMacClean, "_reset");
    // Topics
    JSON_PROPERTY("avty_t", mqttAvailabilityTopic);
    JSON_PROPERTY("cmd_t", "counter/", mqttMacClean, "/", TOPIC_RESET);
    // Entity-related data
    JSON_PROPERTY("icon", "mdi:restart");
    JSON_PROPERTY("pl_prs", MQTT_RESET);
  JSON_END();
  
  CONCAT_TOPIC(SETTINGS_MQTT_HA.discoverytopic, "/button/counter_", mqttMacClean, "_reset/button/config");
  PUBLISH();

}

void mqttHaAutoDisconvery() {
  mqttHaAutoDiscoveryLight();
  mqttHaAutoDiscoveryAutobrightness();
  mqttHaAutoDiscoveryIlluminance();
  mqttHaAutoDiscoveryReset();
}

/* Setting custom - mqttha.friendlyname */
void svm_mqttha_friendlyname_factory(SettingValueManager* svm) {
  char* friendlyname = (char*)svm->_valuePtr;
  strcpy(friendlyname, "counter_");
  strcat(friendlyname, mqttMacClean);
}

void svm_mqttha_friendlyname_afterSet(SettingValueManager* svm, const char* argument, void(*outputFunc)(const char* message))  {
  if (strcmp(argument, "_def_") == 0) {
    svm_mqttha_friendlyname_factory(svm);
    outputFunc("mqttha.friendlyname set to factory default: ");
    outputFunc(SETTINGS_MQTT_HA.friendlyname);
	outputFunc("\r\n");
  }
}

void svm_mqttha_friendlyname_initializer(SettingValueManager* svm)  {
  svm->_ovrFactory = svm_mqttha_friendlyname_factory;
  svm->_hookAfterSet = svm_mqttha_friendlyname_afterSet;
}

/* Settings */
SettingsMqttHa SETTINGS_MQTT_HA;

SettingValueManagerString SVM_MQTTHA_DISCOVERYTOPIC(SETTING_MQTT_HA_DISCOVERYTOPIC, SETTINGS_MQTT_HA.discoverytopic, "homeassistant", -1, MQTT_HA_DISCOVERYTOPIC_MAXLENGTH);
SettingValueManagerString SVM_MQTTHA_FRIENDLYNAME(SETTING_MQTT_HA_FRIENDLYNAME, SETTINGS_MQTT_HA.friendlyname, "counter_xxxxxxxxxxxx", -1, MQTT_HA_FRIENDLYNAME_MAXLENGTH, svm_mqttha_friendlyname_initializer);

SettingValueManager* SM_MQTT_HA_MEMBERS[] = { &SVM_MQTTHA_DISCOVERYTOPIC, &SVM_MQTTHA_FRIENDLYNAME, NULL };
SettingsManager SM_MQTT_HA(SETTING_MQTT_HA, LONGTIME, SETTINGS_DATA(SETTINGS_MQTT_HA), SM_MQTT_HA_MEMBERS);
