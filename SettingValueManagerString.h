#ifndef SETTINGVALUEMANAGERSTRING_H_
#define SETTINGVALUEMANAGERSTRING_H_

#include "SettingValueManager.h"

class SettingValueManagerString : public SettingValueManager {
private:
	const char* _factory;
	int _minLength;
	int _maxLength;
public:
	SettingValueManagerString(const char* name, char* valuePtr, const char* factory, int minLength, int maxLength, void(*initializer)(SettingValueManager* svm) = NULL);
	virtual SettingHandleStatus handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message));
protected:
	virtual void toString(char* buffer);
	virtual void factoryDefaultImpl();
};

#endif