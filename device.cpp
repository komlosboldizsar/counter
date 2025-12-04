#include "device.h"
#include "main.h"
#include <Arduino.h>
#include "settings.h"
#include "Esp.h"

void deviceIntroduce(void(*outputFunc)(const char* message)) {
  outputFunc("***** CLOCK 1.0\" [");
  outputFunc(SETTINGS_DEVICE.name);
  outputFunc("] *****\r\n");
}

static char hexTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

/* Setting custom - device.name */
void svm_device_name_factory(SettingValueManager* svm) {
  char* name = (char*)svm->_valuePtr;
  strcpy(name, "cnt-");
  int i = strlen(name);
  uint64_t mac = ESP.getEfuseMac();
  mac >>= (4*8);
  for (int i = 4; i < 8; i += 2) {
    name[i+0] = hexTable[(mac & 0xF0) >> 4];
    name[i+1] = hexTable[mac & 0x0F];
    mac >>= 8;
  }
  name[8] = '\0';
}

void svm_device_name_afterSet(SettingValueManager* svm, const char* argument, void(*outputFunc)(const char* message))  {
  if (strcmp(argument, "_def_") == 0) {
    svm_device_name_factory(svm);
    outputFunc("device.name set to factory default: ");
    outputFunc(SETTINGS_DEVICE.name);
	  outputFunc("\r\n");
  }
}

void svm_device_name_initializer(SettingValueManager* svm)  {
  svm->_ovrFactory = svm_device_name_factory;
  svm->_hookAfterSet = svm_device_name_afterSet;
}

/* Settings */
SettingsDevice SETTINGS_DEVICE;

SettingValueManagerString SVM_DEVICE_NAME(SETTING_DEVICE_NAME, SETTINGS_DEVICE.name, "CNTR", -1, DEVICE_NAME_MAXLENGTH, svm_device_name_initializer);
SettingValueManagerBool SVM_DEVICE_SHOWMAC(SETTING_DEVICE_SHOWMAC, &SETTINGS_DEVICE.showmac, true);

SettingValueManager* SM_DEVICE_MEMBERS[] = { &SVM_DEVICE_NAME, &SVM_DEVICE_SHOWMAC, NULL };
SettingsManager SM_DEVICE(SETTING_DEVICE, LONGTIME, SETTINGS_DATA(SETTINGS_DEVICE), SM_DEVICE_MEMBERS);