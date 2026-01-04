#ifndef MQTT_STRINGS_H_
#define MQTT_STRINGS_H_

#include "display.h"

extern const char TOPIC_BASE[];
extern const char TOPIC_AVAILABILITY[];
extern const char TOPIC_AUTOBRIGHTNESS_SET[];
extern const char TOPIC_AUTOBRIGHTNESS_STATE[];
extern const char TOPIC_BRIGHTNESS_SET[];
extern const char TOPIC_BRIGHTNESS_STATE[];
extern const char TOPIC_SWITCH_SET[];
extern const char TOPIC_SWITCH_STATE[];
extern const char TOPIC_ILLUMINANCE_STATE[];
extern const char TOPIC_RESET[];
extern const char TOPIC_TEXT_SET[];
extern const char TOPIC_TEXT_STATE[];
extern const char TOPIC_BLINK_SET[];
extern const char TOPIC_BLINK_STATE[];
extern const char TOPIC_BLINKSPEED_SET[];
extern const char TOPIC_BLINKSPEED_STATE[];
extern const char TOPIC_BLINKPHASE_SET[];
extern const char TOPIC_BLINKPHASE_STATE[];
extern const char TOPIC_BLINKDUTYCYCLE_SET[];
extern const char TOPIC_BLINKDUTYCYCLE_STATE[];

extern const char MQTT_ON[];
extern const char MQTT_OFF[];
extern const char MQTT_ONLINE[];
extern const char MQTT_OFFLINE[];
extern const char MQTT_RESET[];

#define MQTT_BLINKOPTION_MAXLENGTH 8
extern const char MQTT_BLINKSPEEDS[DBS___end][MQTT_BLINKOPTION_MAXLENGTH];
extern const char MQTT_BLINKPHASES[DBP___end][MQTT_BLINKOPTION_MAXLENGTH];
extern const char MQTT_BLINKDUTYCYCLES[DBDC___end][MQTT_BLINKOPTION_MAXLENGTH];

extern char mqttMacClean[];

extern int mqttBaseTopicLength;

extern char mqttBaseTopic[];
extern char mqttBaseTopicWildcard[];
extern char mqttAvailabilityTopic[];
extern char mqttAutobrightnessStateTopic[];
extern char mqttBrightnessStateTopic[];
extern char mqttSwitchStateTopic[];
extern char mqttIlluminanceStateTopic[];

void mqttMacGenerate();
void mqttStringsGenerate();

#endif