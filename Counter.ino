#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "main.h"
#include "pins.h"
#include "serial.h"
#include "settings.h"
#include "display.h"
#include "brightness.h"
#include "mywifi.h"
#include "improv.h"

WiFiClient espClient;
PubSubClient client("mqtt_broker", 1883, espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

unsigned long programStart;

void setup() {
  serialInit();
  Serial.println("");
  Serial.println("***** COUNTER\" *****");
  settingsInit();
  Serial.print("Shown name is [");
  Serial.print(SETTINGS.DEVICE.name);
  Serial.println("]");
  displayInit();
  improvInit();
  wifiInit();

  /*while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }*/

  client.setCallback(callback);

  programStart = millis();
}

char* basetopic = "counter";

void displayForDisplay(int idx, const char* message) {

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

void callback(char* topic, byte* message, unsigned int length) {

  #ifdef DEBUG_MQTT
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  #endif
  
  char strBuffer[129];
  strncpy(strBuffer, (char*)message, 128);
  strBuffer[length > 128 ? 128 : length] = '\0';
  Serial.println(strBuffer);

  int basetopicLength = strlen(basetopic);
  int topicLength = strlen(topic);

  if (strncmp(topic, basetopic, basetopicLength) == 0) {

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
      displayForDisplay(idx, strBuffer);
      #endif

    }

  }

  
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(SETTINGS.DEVICE.name, "mqtt_user", "mqtt_password")) {
      Serial.println("connected");
      client.subscribe("counter/+");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

int t = 0;

void loop() {

  unsigned long loopStart = millis();

  serialRead();
  brightnessUpdate();

  if ((t > 200) && !client.connected()) {
    reconnect();
  }
  client.loop();

  bool wifiChanged = false;
  improvMainLoop(loopStart, &wifiChanged);
  if (wifiChanged) {
    delay(500);
    ESP.restart();
  }

  unsigned long loopEnd = millis();
  unsigned long loopElapsed = loopEnd - loopStart;
  unsigned long waitTime = MAIN_LOOP_TIME - loopElapsed;
  if ((waitTime < 0) || (waitTime > MAIN_LOOP_TIME))
    waitTime = 5;
  delay(100);
  t++;
  
}