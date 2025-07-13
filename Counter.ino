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
  mqttInit();

  programStart = millis();
}

void loop() {

  unsigned long loopStart = millis();

  serialRead();
  brightnessUpdate();
  mqttMainLoop(loopStart);

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
  
}