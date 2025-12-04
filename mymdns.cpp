#include "mdns.h"
#include <Arduino.h>
#include <ESPmDNS.h>
#include "settings.h"
#include "device.h"

void mymdnsInit() {
  MDNS.begin(SETTINGS_DEVICE.name);
}
