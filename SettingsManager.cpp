#include "SettingsManager.h"
#include "utils.h"
#include "main.h"
#include <FS.h>
#include <LittleFS.h>

const char SETTING_TYPE_LONGTIME[] = "longtime";
const char SETTING_TYPE_RUNTIME[] = "runtime";

#define GEN_FILEPATH()                      CONCAT(filePath, 64, "/", (this->_type == RUNTIME) ? SETTING_TYPE_RUNTIME : SETTING_TYPE_LONGTIME, "/", this->_name)

#define LOOP_SETTINGS_VALUES_BEGIN()		    int memberIndex = 0; \
											                      SettingValueManager* member; \
											                      while((member = this->_members[memberIndex]) != NULL) {
#define LOOP_SETTINGS_VALUES_END()				    memberIndex++; \
											                      }

#define F_OPEN(MODE)                        File file = LittleFS.open(filePath, MODE)
#define F_OPER(OPER)                        file.OPER((uint8_t*)this->_data, this->_dataSize)
#define F_CLOSE()                           file.close()
#define F_READ()                            F_OPEN("r"); F_OPER(read); F_CLOSE();
#define F_WRITE()                           F_OPEN("w"); F_OPER(write); F_CLOSE();

#define D_CREATE(NAME)                      CONCAT(dirPath, 64, "/", NAME); \
                                            if (!LittleFS.exists(dirPath)) \
                                              LittleFS.mkdir(dirPath);

SettingsManager::SettingsManager(const char* name, SettingType type, void* data, int dataSize, SettingValueManager** members):
	_name(name),
	_type(type),
	_data(data),
	_dataSize(dataSize),
	_members(members)
{
  LOOP_SETTINGS_VALUES_BEGIN()
    member->bindOwner(this);
  LOOP_SETTINGS_VALUES_END()
}

void SettingsManager::init() {
  LittleFS.begin(true);
  { D_CREATE(SETTING_TYPE_LONGTIME); }
  { D_CREATE(SETTING_TYPE_RUNTIME); }
}

void SettingsManager::load() {
  GEN_FILEPATH();
  if (LittleFS.exists(filePath)) {
    F_READ();
  } else {
    factoryValues();
    F_WRITE();
  }
}

void SettingsManager::save() {
  GEN_FILEPATH();
  F_WRITE();
}

void SettingsManager::factoryValues() {
  LOOP_SETTINGS_VALUES_BEGIN()
    member->factory();
  LOOP_SETTINGS_VALUES_END()
}

void SettingsManager::dumpValues(void(*outputFunc)(const char* message)) {
	
  char line[64];
  memset(line, '\0', 64);
  strcat(line, "_ ");
  strncat(line, this->_name, 16);
  strcat(line, ": \r\n");
  outputFunc(line);
  
  LOOP_SETTINGS_VALUES_BEGIN()
    member->dumpValue(outputFunc);
  LOOP_SETTINGS_VALUES_END()
	
}

void SettingsManager::exportValues(void(*outputFunc)(const char* message)) {
  LOOP_SETTINGS_VALUES_BEGIN()
    member->exportValue(outputFunc);
  LOOP_SETTINGS_VALUES_END()
}

bool SettingsManager::receiveCommand(const char* subCommand, const char* argument, void(*outputFunc)(const char* message)) {
  SettingHandleStatus handleStatus = NOT_HANDLED;
  LOOP_SETTINGS_VALUES_BEGIN()
    handleStatus = member->handle(subCommand, argument, outputFunc);
	if (handleStatus != NOT_HANDLED)
		return handleStatus;
  LOOP_SETTINGS_VALUES_END()
  return NOT_HANDLED;
}

void SettingsManager::runtimeChanged() {
  this->_runtimeLastChangeSince = 0;
}

const int SETTINGS_RUNTIME_SAVE_TIME = (1000/MAIN_LOOP_TIME) * 5;

void SettingsManager::runtimeMainLoop() {
  if (this->_runtimeLastChangeSince == SETTINGS_RUNTIME_SAVE_TIME)
    save();
  if ((this->_runtimeLastChangeSince >= 0) && (this->_runtimeLastChangeSince <= SETTINGS_RUNTIME_SAVE_TIME))
    this->_runtimeLastChangeSince++;
}