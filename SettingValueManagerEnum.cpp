#include "SettingValueManagerEnum.h"

SettingValueManagerEnum::SettingValueManagerEnum(const char* name, int* valuePtr, int factory, const char** options, void(*initializer)(SettingValueManager* svm))
	: SettingValueManager(name, (void*)valuePtr, initializer),
	  _factory(factory),
	  _options(options)
{
  const char** ptrOption = options;
  while (*ptrOption != NULL)
    ptrOption++;
  this->_numOptions = ptrOption - options;
}

void SettingValueManagerEnum::factoryDefaultImpl() {
  *((int*)this->_valuePtr) = this->_factory;
}

SettingHandleStatus SettingValueManagerEnum::handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message)) {

  if (strcmp(subCommand, this->_name) != 0)
    return NOT_HANDLED;

  if (!preValidate(argument, outputFunc))
	return HANDLED_ERROR;

  int selectedOption = -1;
  for (int i = 0; i < this->_numOptions; i++)
    if (strcmp(argument, this->_options[i]) == 0)
	  selectedOption = i;
  
  if (selectedOption == -1) {
    error("unknown option", outputFunc);
    return HANDLED_ERROR;
  }
  
  beforeSet(argument, outputFunc);
  *((int*)this->_valuePtr) = selectedOption;
  afterSet(argument, outputFunc);
  done(argument, outputFunc);
  return HANDLED_SUCCESS;
  
}

void SettingValueManagerEnum::toString(char* buffer) {
  int value = *((int*)this->_valuePtr);
  strcpy(buffer, ((value >= 0) && (value < this->_numOptions)) ? this->_options[value] : "!invalid");
}