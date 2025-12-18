#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "pins.h"
#include "serial.h"
#include "settings.h"
#include "display.h"
#include "brightness.h"
#include "mywifi.h"
#include "improv.h"
#include "mqtt.h"
#include "mqtt-strings.h"
#include "device.h"
#include "mymdns.h"
#include "ota.h"
#include "telnet.h"

unsigned long programStart;
unsigned long loopCounter = 0;

void setup() {
  mqttMacGenerate();
  serialInit();
  Serial.println("");
  Serial.println("");
  settingsInit();
  settingsLoad();
  deviceIntroduce(serialWrite);
  displayInit();
  brightnessInit();
  improvInit();
  wifiInit();
  mqttInit();
  mymdnsInit();
  otaInit();
  telnetInit();
  programStart = millis();
}

int wifiBlinkTimeCounter = 0;
DotForcing wifiBlinkState = DOT_NO_FORCE;

bool loopWifiBlink(DotForcing* pattern) {
  if (WiFi.status() != WL_CONNECTED) {
    wifiBlinkTimeCounter++;
    if (wifiBlinkTimeCounter >= LOOP_PER_SECOND/2-1) {
      wifiBlinkTimeCounter = 0;
      wifiBlinkState = (wifiBlinkState == DOT_FORCE_ON) ? DOT_FORCE_OFF : DOT_FORCE_ON;
    }
  } else {
    wifiBlinkTimeCounter = 0;
    wifiBlinkState = DOT_NO_FORCE;
  }
  for (int i = 0; i < DIGITS_PER_DISPLAY; i++)
    pattern[i] = (i == DIGITS_PER_DISPLAY-1) ? wifiBlinkState : DOT_NO_FORCE;
  return (wifiBlinkState != DOT_NO_FORCE);
}

int mqttBlinkTimeCounter = 0;
int mqttBlinkPhaseCounter = 0;

bool loopMqttBlink(DotForcing* pattern) {
  if (!mqttIsConnected()) {
    mqttBlinkTimeCounter++;
    if (mqttBlinkTimeCounter >= LOOP_PER_SECOND-1) {
      mqttBlinkTimeCounter = 0;
      mqttBlinkPhaseCounter++;
      if (mqttBlinkPhaseCounter > 2*DIGITS_PER_DISPLAY-2-1)
        mqttBlinkPhaseCounter = 0;
    }
    for (int i = 0; i < DIGITS_PER_DISPLAY; i++) {
      bool a = (mqttBlinkPhaseCounter == i);
      bool b = (mqttBlinkPhaseCounter == (2*DIGITS_PER_DISPLAY-2-i));
      pattern[DIGITS_PER_DISPLAY-1-i] = (a || b) ? DOT_FORCE_ON : DOT_FORCE_OFF;
    }
    return true;
  } else {
    return false;
  }
}

void loop() {

  unsigned long loopStart = millis();

  DotForcing dotForcingPattern[4];
  if (loopWifiBlink(dotForcingPattern) || loopMqttBlink(dotForcingPattern))
    for (int i = 0; i < DIGITS_PER_DISPLAY; i++)
      displaySetDotForcing(0, i, dotForcingPattern[i]);
  else
    displayClearDotForcing(0);

  serialRead();
  displayMainLoop();
  brightnessUpdate(loopCounter & 0x0F == 0x0F);
  mqttMainLoop();
  settingsMainLoop();
  otaMainLoop();
  telnetMainLoop();

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
  delay(waitTime);

  loopCounter++;
  
}