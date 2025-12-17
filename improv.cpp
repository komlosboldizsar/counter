#include "improv.h"
#include "NimBLEDevice.h"
#include "mywifi.h"
#include "wifi.h"
#include "device.h"

#define DEBUG_IMPROV

#define IMPROV_CS_AUTHREQ           0x01
#define IMPROV_CS_AUTHORIZED        0x02
#define IMPROV_CS_PROVISIONING      0x03
#define IMPROV_CS_PROVISIONED       0x04

#define IMPROV_ES_NOERROR           0x00
#define IMPROV_ES_INVALIDRPC        0x01
#define IMPROV_ES_UNKNOWNRPC        0x02
#define IMPROV_ES_UNABLETOCONNECT   0x03
#define IMPROV_ES_NOTAUTHORIZED     0x04
#define IMPROV_ES_UNKNOWNERR        0xFF

BLEServer *improvServer = NULL;
BLECharacteristic *improvCharacteristicCurrentState, *improvCharacteristicErrorState, *improvCharacteristicRpcCommand, *improvCharacteristicRpcResult, *improvCharacteristicCaps;

bool improvDeviceConnected = false;
bool improvDldDeviceConnected = false;
bool improvWifiChanged = false;
uint8_t txValue = 0;

uint8_t improvChecksum(uint8_t* buffer, uint8_t length) {
  uint8_t result = 0;
  for (int i = 0; i < length; i++)
    result = (uint8_t)(result + buffer[i]);
  return result;
}

void improvSetCurrentState(uint8_t currentState) {
  int __currentState = currentState;
  improvCharacteristicCurrentState->setValue(__currentState);
}

void improvSetErrorState(uint8_t errorState) {
  int __errorState = errorState;
  improvCharacteristicErrorState->setValue(__errorState);
}

class ImprovServerCallbacks : public BLEServerCallbacks {

  void onConnect(BLEServer *server) {
    #ifdef DEBUG_IMPROV
    Serial.println("BLE connected.");
    #endif
    improvDeviceConnected = true;
  }

  void onDisconnect(BLEServer *server) {
    #ifdef DEBUG_IMPROV
    Serial.println("BLE disconnected.");
    #endif
    improvDeviceConnected = false;
    improvServer->startAdvertising();
  }

};

class ImprovCharacteristicRpcCommandCallbacks : public BLECharacteristicCallbacks {

  int expectedFullStringLength = -1;
  char receivedFullString[128+1];
  int receivedFullStringIndex = 0;
  bool receivedFullStringOverflow = false;

  void processRpcSendWifiSettingsCommand() {

    #ifdef DEBUG_IMPROV
    Serial.print("BLE - send WiFi settings command, ");
    #endif

    improvSetCurrentState(IMPROV_CS_PROVISIONING);

    char wifiSsid[WIFI_SSID_MAXLENGTH+1], wifiPassword[WIFI_PASSWORD_MAXLENGTH+1];

    if (receivedFullStringIndex < 3) { // no SSID length field
      improvSetErrorState(IMPROV_ES_INVALIDRPC);
      improvSetCurrentState(IMPROV_CS_AUTHORIZED);
      #ifdef DEBUG_IMPROV
      Serial.print("error: too short");
      #endif
      return;
    }

    int wifiSsidLength = receivedFullString[2];

    if (wifiSsidLength > WIFI_SSID_MAXLENGTH) { // SSID too long
      improvSetErrorState(IMPROV_ES_INVALIDRPC);
      improvSetCurrentState(IMPROV_CS_AUTHORIZED);
      #ifdef DEBUG_IMPROV
      Serial.print("error: too long SSID");
      #endif
      return;
    }

    if (receivedFullStringIndex < wifiSsidLength+3+1) { // SSID not long enough
      improvSetErrorState(IMPROV_ES_INVALIDRPC);
      improvSetCurrentState(IMPROV_CS_AUTHORIZED);
      #ifdef DEBUG_IMPROV
      Serial.print("error: too short");
      #endif
      return;
    }

    memcpy(wifiSsid, receivedFullString+3, wifiSsidLength);
    wifiSsid[wifiSsidLength] = '\0';

    if (receivedFullStringIndex < wifiSsidLength+3+1+1) { // no PW length field
      improvSetErrorState(IMPROV_ES_INVALIDRPC);
      improvSetCurrentState(IMPROV_CS_AUTHORIZED);
      #ifdef DEBUG_IMPROV
      Serial.print("error: too short");
      #endif
      return;
    }

    int wifiPasswordLength = receivedFullString[wifiSsidLength+3];

    if (wifiPasswordLength > WIFI_PASSWORD_MAXLENGTH) { // SSID too long
      improvSetErrorState(IMPROV_ES_INVALIDRPC);
      improvSetCurrentState(IMPROV_CS_AUTHORIZED);
      #ifdef DEBUG_IMPROV
      Serial.print("error: too long PW");
      #endif
      return;
    }

    if (receivedFullStringIndex < wifiSsidLength+wifiPasswordLength+4+1) { // PW not long enough
      improvSetErrorState(IMPROV_ES_INVALIDRPC);
      improvSetCurrentState(IMPROV_CS_AUTHORIZED);
      #ifdef DEBUG_IMPROV
      Serial.print("error: too short");
      #endif
      return;
    }

    memcpy(wifiPassword, receivedFullString+wifiSsidLength+4, wifiPasswordLength);
    wifiPassword[wifiPasswordLength] = '\0';

    strcpy(SETTINGS_WIFI.ssid, wifiSsid);
    strcpy(SETTINGS_WIFI.password, wifiPassword);
    settingsSave();
    improvWifiChanged = true;

    improvSetCurrentState(IMPROV_CS_PROVISIONED);
    improvCharacteristicCurrentState->notify();

    uint8_t rpcResultBuffer[] = { 0x01, 0x00, 0x00 };
    rpcResultBuffer[2] = improvChecksum(rpcResultBuffer, 2);
    improvCharacteristicRpcResult->setValue(rpcResultBuffer, 3);
    improvCharacteristicRpcResult->notify();

    #ifdef DEBUG_IMPROV
    Serial.print("SSID: [");
    Serial.print(wifiSsid);
    Serial.print("], password: [");
    Serial.print(wifiPassword);
    Serial.println("]");
    #endif

  }

  void processRpcIdentifyCommand() {

  }

  void processRpc() {

    #ifdef DEBUG_IMPROV
    Serial.print("BLE - full RPC command: ");
    Serial.println(receivedFullString);
    #endif

    uint8_t receivedChecksum = receivedFullString[receivedFullStringIndex-2];
    uint8_t calculatedChecksum = improvChecksum((uint8_t*)receivedFullString, receivedFullStringIndex-2);
    if (receivedChecksum != calculatedChecksum) {
      #ifdef DEBUG_IMPROV
      Serial.print("BLE - RPC command checksum error. Received: [0x");
      Serial.print(receivedChecksum, HEX);
      Serial.print("], calculated: [0x");
      Serial.print(calculatedChecksum, HEX);
      Serial.println("]");
      #endif
      improvSetErrorState(IMPROV_ES_INVALIDRPC);
      improvSetCurrentState(IMPROV_CS_AUTHORIZED);
      return;
    }
    
    if (receivedFullString[0] == 0x01) {
      improvSetErrorState(IMPROV_ES_NOERROR);
      processRpcSendWifiSettingsCommand();
      return;
    }

    if (receivedFullString[0] == 0x02) {
      improvSetErrorState(IMPROV_ES_NOERROR);
      processRpcIdentifyCommand();
      return;
    }

    #ifdef DEBUG_IMPROV
    Serial.println("BLE - unknown RPC command");
    #endif
    improvSetErrorState(IMPROV_ES_UNKNOWNRPC);
    improvSetCurrentState(IMPROV_CS_AUTHORIZED);

  }

  void onWrite(BLECharacteristic *characteristic) {

    String rxValue = characteristic->getValue();

    int rxLength = rxValue.length();
    if ((expectedFullStringLength == -1) && (rxLength >= 2)) {
      receivedFullStringIndex = 0;
      expectedFullStringLength = rxValue[1];
      #ifdef DEBUG_IMPROV
      Serial.print("BLE - beginning RPC command, expected length: ");
      Serial.println(expectedFullStringLength, DEC);
      #endif
    }

    if (rxLength > 0) {
      #ifdef DEBUG_IMPROV
      Serial.print("BLE - received RPC command part: ");
      for (int i = 0; i < rxLength; i++) {
        Serial.print(rxValue[i]);
      }
      Serial.println();
      #endif
    }

    for (int i = 0; i < rxLength; i++) {
      if (receivedFullStringIndex < 128) {
        receivedFullString[receivedFullStringIndex++] = rxValue[i];
      }
    }

    if ((expectedFullStringLength != -1) && (receivedFullStringIndex >= expectedFullStringLength + 3)) {
      receivedFullString[receivedFullStringIndex++] = '\0';
      processRpc();
      expectedFullStringLength = -1;
    }
    
  }
  
};

void improvInit() {
  
  BLEDevice::init(SETTINGS_DEVICE.name);
  improvServer = BLEDevice::createServer();
  improvServer->setCallbacks(new ImprovServerCallbacks());
  BLEService *improvService = improvServer->createService("00467768-6228-2272-4663-277478268000");

  // Characteristic: CURRENT STATE
  improvCharacteristicCurrentState = improvService->createCharacteristic("00467768-6228-2272-4663-277478268001", NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
  improvSetCurrentState(IMPROV_CS_AUTHORIZED);
  improvCharacteristicCurrentState->addDescriptor(new NimBLE2904());

  // Characteristic: ERROR STATE
  improvCharacteristicErrorState = improvService->createCharacteristic("00467768-6228-2272-4663-277478268002", NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
  improvSetErrorState(IMPROV_ES_NOERROR);
  improvCharacteristicErrorState->addDescriptor(new NimBLE2904());

  // Characteristic: RPC COMMAND
  improvCharacteristicRpcCommand = improvService->createCharacteristic("00467768-6228-2272-4663-277478268003", NIMBLE_PROPERTY::WRITE);
  improvCharacteristicRpcCommand->setCallbacks(new ImprovCharacteristicRpcCommandCallbacks());
  improvCharacteristicRpcCommand->addDescriptor(new NimBLE2904());

  // Characteristic: RPC RESULT
  improvCharacteristicRpcResult = improvService->createCharacteristic("00467768-6228-2272-4663-277478268004", NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
  improvCharacteristicRpcResult->addDescriptor(new NimBLE2904());

  // Characteristic: CAPS
  improvCharacteristicCaps = improvService->createCharacteristic("00467768-6228-2272-4663-277478268005", NIMBLE_PROPERTY::READ);
  int improvCharacteristicCapsValue = 1;
  improvCharacteristicCaps->setValue(improvCharacteristicCapsValue);
  improvCharacteristicCaps->addDescriptor(new NimBLE2904());

  improvService->start();

  uint8_t bleSvcData[] = {1, 1, 0xFE, 0xFE, 0xFE, 0xFE};
  BLEAdvertisementData bleAdvData;
  bleAdvData.setServiceData(NimBLEUUID((uint8_t*)"4677", 4), bleSvcData, sizeof(bleSvcData)/sizeof(uint8_t));
  improvServer->getAdvertising()->addServiceUUID("00467768-6228-2272-4663-277478268000");
  improvServer->getAdvertising()->start();
  improvServer->getAdvertising()->setAdvertisementData(bleAdvData);

}

void improvMainLoop(unsigned long now, bool* wifiChanged) {
  if (improvWifiChanged) {
    *wifiChanged = true;
    improvWifiChanged = false;
  }
}
