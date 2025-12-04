#ifndef SETTINGVALUEMANAGERBOOL_H_
#define SETTINGVALUEMANAGERBOOL_H_

#include "SettingValueManager.h"

class SettingValueManagerBool : public SettingValueManager {
private:
	bool _factory;
public:
	SettingValueManagerBool(const char* name, bool* valuePtr, bool factory, void(*initializer)(SettingValueManager* svm) = NULL);
	virtual SettingHandleStatus handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message));
protected:
	virtual void toString(char* buffer);
	virtual void factoryDefaultImpl();
};

#endif