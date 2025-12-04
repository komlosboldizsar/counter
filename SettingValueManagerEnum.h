#ifndef SETTINGVALUEMANAGERENUM_H_
#define SETTINGVALUEMANAGERENUM_H_

#include "SettingValueManager.h"

class SettingValueManagerEnum : public SettingValueManager {
private:
	int _factory;
	const char** _options;
	int _numOptions;
public:
	SettingValueManagerEnum(const char* name, int* valuePtr, int factory, const char** options, void(*initializer)(SettingValueManager* svm) = NULL);
	virtual SettingHandleStatus handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message));
protected:
	virtual void toString(char* buffer);
	virtual void factoryDefaultImpl();
};

#endif