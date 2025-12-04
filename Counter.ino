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
#include "device.h"
#include "mymdns.h"
#include "telnet.h"

unsigned long programStart;
unsigned long loopCounter = 0;

void setup() {
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
  telnetInit();
  programStart = millis();
}

void loop() {

  unsigned long loopStart = millis();

  serialRead();
  brightnessUpdate(loopCounter & 0x0F == 0x0F);
  settingsMainLoop();
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