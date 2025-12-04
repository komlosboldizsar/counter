#include "SettingValueManagerBool.h"

const char* SETTING_BOOL_YES = "yes";
const char* SETTING_BOOL_NO = "no";

SettingValueManagerBool::SettingValueManagerBool(const char* name, bool* valuePtr, bool factory, void(*initializer)(SettingValueManager* svm))
	: SettingValueManager(name, (void*)valuePtr, initializer),
	  _factory(factory)
{ }

void SettingValueManagerBool::factoryDefaultImpl() {
  *((bool*)this->_valuePtr) = this->_factory;
}

SettingHandleStatus SettingValueManagerBool::handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message)) {
	
  if (strcmp(subCommand, this->_name) != 0)
    return NOT_HANDLED;

  if (!preValidate(argument, outputFunc))
	  return HANDLED_ERROR;

  if (strcmp(argument, SETTING_BOOL_YES) == 0) {
    *((bool*)this->_valuePtr) = true;
    beforeSet(argument, outputFunc);
    done(argument, outputFunc);
    afterSet(argument, outputFunc);
    return HANDLED_SUCCESS;
  }
  
  if (strcmp(argument, SETTING_BOOL_NO) == 0) {
    *((bool*)this->_valuePtr) = false;
    beforeSet(argument, outputFunc);
    done(argument, outputFunc);
    afterSet(argument, outputFunc);
    return HANDLED_SUCCESS;
  }

  error("invalid value (yes/no)", outputFunc);
  return HANDLED_ERROR;
	
}

void SettingValueManagerBool::toString(char* buffer) {
  strcpy(buffer, *((bool*)this->_valuePtr) ? SETTING_BOOL_YES : SETTING_BOOL_NO);
}
