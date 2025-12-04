#ifndef SETTINGVALUEMANAGERINT_H_
#define SETTINGVALUEMANAGERINT_H_

#include "SettingValueManager.h"

class SettingValueManagerInt : public SettingValueManager {
public:
	void (*_hookGetLimits)(SettingValueManager* svm, int* valueMin, int* valueMax);
	bool (*_hookValidLimits)(SettingValueManager* svm, int value, char* error);
private:
	int _factory;
	int _valueMin;
	int _valueMax;
public:
	SettingValueManagerInt(const char* name, int* valuePtr, int factory, int valueMin, int valueMax, void(*initializer)(SettingValueManager* svm) = NULL);
	virtual SettingHandleStatus handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message));
protected:
	virtual void toString(char* buffer);
	virtual void factoryDefaultImpl();
};

#endif