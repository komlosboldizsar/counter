#include "SettingValueManagerInt.h"

SettingValueManagerInt::SettingValueManagerInt(const char* name, int* valuePtr, int factory, int valueMin, int valueMax, void(*initializer)(SettingValueManager* svm))
	: SettingValueManager(name, (void*)valuePtr, initializer),
	  _factory(factory),
	  _valueMin(valueMin),
	  _valueMax(valueMax)
{ }

void SettingValueManagerInt::factoryDefaultImpl() {
	*((int*)this->_valuePtr) = this->_factory;
}

SettingHandleStatus SettingValueManagerInt::handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message)) {
	
  if (strcmp(subCommand, this->_name) != 0)
    return NOT_HANDLED;

  if (!preValidate(argument, outputFunc))
	  return HANDLED_ERROR;

  bool negative = false;
  bool invalid = false;
  long value = 0;

  if (*argument == '-') {
    negative = true;
    argument++;
  } else if (*argument == '+') {
    argument++;
  }

  while (!invalid && (*argument != '\0')) {
    if ((*argument >= '0') && (*argument <= '9')) {
      value *= 10;
      value += *argument - '0';
      argument++;
    } else {
      invalid = true;
    }
  }

  if (negative)
    value *= -1;

  if (invalid) {
    error("invalid integer value", outputFunc);
    return HANDLED_ERROR;
  }
  
  int valueMin = this->_valueMin;
  int valueMax = this->_valueMax;
  if (this->_hookGetLimits != NULL)
    this->_hookGetLimits(this, &valueMin, &valueMax);
  
  if (value < valueMin) {
    error("too small", outputFunc);
    return HANDLED_ERROR;
  }
  
  if (value > valueMax) {
    error("too big", outputFunc);
    return HANDLED_ERROR;
  }
  
  if (this->_hookValidLimits != NULL) {
    char errorMsg[64];
    bool valid = this->_hookValidLimits(this, value, errorMsg);
    if (!valid) {
	  error(errorMsg, outputFunc);
	  return HANDLED_ERROR;
    }
  }
  
  *((int*)this->_valuePtr) = (int)value;
  beforeSet(argument, outputFunc);
  done(argument, outputFunc);
  afterSet(argument, outputFunc);
  return HANDLED_SUCCESS;
	
}

void SettingValueManagerInt::toString(char* buffer) {
	itoa(*((int*)this->_valuePtr), buffer, 10);
}
