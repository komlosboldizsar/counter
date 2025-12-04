#include "SettingValueManagerString.h"

SettingValueManagerString::SettingValueManagerString(const char* name, char* valuePtr, const char* factory, int minLength, int maxLength, void(*initializer)(SettingValueManager* svm))
	: SettingValueManager(name, (void*)valuePtr, initializer),
	  _factory(factory),
	  _minLength(minLength),
	  _maxLength(maxLength)
{ }

void SettingValueManagerString::factoryDefaultImpl() {
	strcpy((char*)this->_valuePtr, this->_factory);
}

SettingHandleStatus SettingValueManagerString::handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message)) {
	
  if (strcmp(subCommand, this->_name) != 0)
    return NOT_HANDLED;

  if (!preValidate(argument, outputFunc))
	  return HANDLED_ERROR;

  if ((this->_minLength > -1) && (strlen(argument) < this->_minLength)) {
    error("too short", outputFunc);
    return HANDLED_ERROR;
  }
  
  if ((this->_maxLength > -1) && (strlen(argument) > this->_maxLength)) {
    error("too long", outputFunc);
    return HANDLED_ERROR;
  }

  beforeSet(argument, outputFunc);
  strcpy((char*)this->_valuePtr, argument);
  afterSet(argument, outputFunc);
  done(argument, outputFunc);
  return HANDLED_SUCCESS;
  
}

void SettingValueManagerString::toString(char* buffer) {
	strcpy(buffer, (char*)this->_valuePtr);
}