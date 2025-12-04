#include "ota.h"
#include "main.h"
#include <Arduino.h>
#include "settings.h"
#include "device.h"
#include <ArduinoOTA.h>

#define DEBUG_OTA(MSG)                Serial.println("[OTA] " MSG )
#define DEBUG_OTA_F(MSG,...)          Serial.printf("[OTA] " MSG "\r\n", __VA_ARGS__)
#define DEBUG_OTA_ERR(ERR)            DEBUG_OTA("Error, meaning: " ERR )

void otaOnStart() {
	DEBUG_OTA("Start");
}

void otaOnEnd() {
	DEBUG_OTA("End");
}

void otaOnProgress(unsigned int progress, unsigned int total) {
	DEBUG_OTA_F("Progress: %u%%\r", (progress / (total / 100)));
}

void otaOnError(ota_error_t error) {
	DEBUG_OTA_F("Error, code: %u", error);
	if (error == OTA_AUTH_ERROR)
		DEBUG_OTA_ERR("auth failed");
	else if (error == OTA_BEGIN_ERROR)
		DEBUG_OTA_ERR("begin failed");
	else if (error == OTA_CONNECT_ERROR)
		DEBUG_OTA_ERR("connect failed");
	else if (error == OTA_RECEIVE_ERROR)
		DEBUG_OTA_ERR("receive failed");
	else if (error == OTA_END_ERROR)
		DEBUG_OTA_ERR("end failed");
	else
		DEBUG_OTA_ERR("unknown");
}

void otaInit() {
	if (!SETTINGS_OTA.enable)
		return;
	ArduinoOTA.onStart(otaOnStart);
	ArduinoOTA.onEnd(otaOnEnd);
	ArduinoOTA.onProgress(otaOnProgress);
	ArduinoOTA.onError(otaOnError);
	ArduinoOTA.setHostname(SETTINGS_DEVICE.name);
	ArduinoOTA.begin();
}

void otaMainLoop() {
	if (!SETTINGS_OTA.enable)
		return;
	ArduinoOTA.handle();
}

/* Settings */

SettingsOta SETTINGS_OTA;

SettingValueManagerBool SVM_OTA_AUTO(SETTING_OTA_ENABLE, &SETTINGS_OTA.enable, true);
SettingValueManagerString SVM_OTA_MANUAL(SETTING_OTA_PASSWORD, SETTINGS_OTA.password, "ota", -1, OTA_PASSWORD_MAXLENGTH);

SettingValueManager* SM_OTA_MEMBERS[] = { &SVM_OTA_AUTO, &SVM_OTA_MANUAL, NULL };
SettingsManager SM_OTA(SETTING_OTA, LONGTIME, SETTINGS_DATA(SETTINGS_OTA), SM_OTA_MEMBERS);
