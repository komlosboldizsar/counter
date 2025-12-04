#include "SettingValueManager.h"
#include "SettingsManager.h"

SettingValueManager::SettingValueManager(const char* name, void* valuePtr, void(*initializer)(SettingValueManager* svm))
  : _name(name),
    _valuePtr(valuePtr)
{
	if (initializer != NULL)
		initializer(this);
}

void SettingValueManager::bindOwner(SettingsManager* owner) {
	if (this->_owner != NULL)
		return;
	this->_owner = owner;	
}

void SettingValueManager::factory() {
	if (this->_ovrFactory != NULL)
		this->_ovrFactory(this);
	else
		factoryDefaultImpl();
}

void SettingValueManager::dumpValue(void(*outputFunc)(const char* message)) {
  if (this->_hidden)
    return;
  char line[64];
  memset(line, '\0', 64);
  strcat(line, "___ ");
  strncat(line, this->_name, 16);
  strcat(line, ": ");
  uint8_t len1 = strlen(line);
  memset(line+len1, ' ', 24 - len1);
  strcat(line, "[");
  char buffer[64];
  toString(buffer);
  strncat(line, buffer, 32);
  strcat(line, "]\r\n");
  outputFunc(line);
}

void SettingValueManager::exportValue(void(*outputFunc)(const char* message)) {
  if (this->_hidden)
    return;
  char line[128];
  memset(line, '\0', 64);
  strncat(line, this->_owner->_name, 16);
  strcat(line, " ");
  strncat(line, this->_name, 16);
  strcat(line, " ");
  char buffer[64];
  toString(buffer);
  strncat(line, buffer, 32);
  strcat(line, "\r\n");
  outputFunc(line);
}

void SettingValueManager::done(const char* argument, void(*outputFunc)(const char* message)) {
  char line[64];
  memset(line, '\0', 64);
  strncat(line, this->_owner->_name, 16);
  strcat(line, ".");
  strncat(line, this->_name, 16);
  strcat(line, " set to: ");
  char buffer[64];
  toString(buffer);
  strncat(line, buffer, 32);
  strcat(line, ".\r\n");
  outputFunc(line);
}

void SettingValueManager::error(const char* errorMessage, void(*outputFunc)(const char* message)) {
  char line[64];
  memset(line, '\0', 64);
  strncat(line, this->_owner->_name, 16);
  strcat(line, ".");
  strncat(line, this->_name, 16);
  strcat(line, " setting error: ");
  strncat(line, errorMessage, 32);
  strcat(line, ".\r\n");
  outputFunc(line);
}

bool SettingValueManager::preValidate(const char* argument, void(*outputFunc)(const char* message)) {
  if (this->_hookPreValidate != NULL) {
  char errorMsg[64];
  bool valid = this->_hookPreValidate(this, argument, errorMsg);
  if (!valid)
    error(errorMsg, outputFunc);
    return valid;
  }
  return true;
}

void SettingValueManager::beforeSet(const char* argument, void(*outputFunc)(const char* message)) {
	if (this->_hookBeforeSet != NULL)
		this->_hookBeforeSet(this, argument, outputFunc);
}

void SettingValueManager::afterSet(const char* argument, void(*outputFunc)(const char* message)) {
	if (this->_hookAfterSet != NULL)
		this->_hookAfterSet(this, argument, outputFunc);
	if (this->_owner->_type == RUNTIME)
		this->_owner->runtimeChanged();
}

void svm_initializer_hidden(SettingValueManager* svm) {
  svm->_hidden = true;
}