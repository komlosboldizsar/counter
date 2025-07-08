#include "device.h"
#include "main.h"
#include <Arduino.h>
#include "settings.h"
#include "Esp.h"

static char deviceCurrentName[DEVICE_NAME_MAXLENGTH+16] = { '\0' };
static char hexTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

void deviceNameFactory() {
  strcpy(SETTINGS.DEVICE.name, "Cnt-");
  int i = strlen(SETTINGS.DEVICE.name);
  uint64_t mac = ESP.getEfuseMac();
  mac >>= (4*8);
  for (int i = 4; i < 8; i += 2) {
    SETTINGS.DEVICE.name[i+0] = hexTable[mac & 0xF0];
    SETTINGS.DEVICE.name[i+1] = hexTable[mac & 0x0F];
    mac >>= 8;
  }
  SETTINGS.DEVICE.name[8] = '\0';
}

void deviceSettingsFactory() {
  deviceNameFactory();
}

void deviceSettingsDump() {
  settingsDumpPartStart(SETTING_DEVICE);
  settingsDumpValueString(SETTING_DEVICE_NAME, SETTINGS.DEVICE.name);
}

bool deviceReceiveCommand(const char* subCommand, const char* argument) {

  bool handled = false;
  
  handled = handleSubcommandString(SETTING_DEVICE, SETTING_DEVICE_NAME, SETTINGS.DEVICE.name, -1, DEVICE_NAME_MAXLENGTH, subCommand, argument);
  if (handled) {
    if (strcmp(SETTINGS.DEVICE.name, "_default_") == 0) {
      deviceNameFactory();
      Serial.print("device.name set to factory default: ");
      Serial.println(SETTINGS.DEVICE.name);
    }
    return true;
  }

  return false;
  
}