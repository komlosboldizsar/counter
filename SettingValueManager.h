#ifndef SETTINGVALUEMANAGER_H_
#define SETTINGVALUEMANAGER_H_

#include <Arduino.h>

enum SettingHandleStatus {
  NOT_HANDLED,
  HANDLED_SUCCESS,
  HANDLED_ERROR
};

class SettingsManager;

class SettingValueManager {
public:
	const char* _name;
	void* _valuePtr;
	SettingsManager* _owner;
	bool _hidden;
	void (*_ovrFactory)(SettingValueManager* svm);
	bool (*_hookPreValidate)(SettingValueManager* svm, const char* argument, char* error);
	void (*_hookBeforeSet)(SettingValueManager* svm, const char* argument, void(*outputFunc)(const char* message));
	void (*_hookAfterSet)(SettingValueManager* svm, const char* argument, void(*outputFunc)(const char* message));
public:
	SettingValueManager(const char* name, void* valuePtr, void(*initializer)(SettingValueManager* svm) = NULL);
    void bindOwner(SettingsManager* owner);
	void factory();
	virtual void factoryDefaultImpl() = 0;
	void dumpValue(void(*outputFunc)(const char* message));
	void exportValue(void(*outputFunc)(const char* message));
	virtual SettingHandleStatus handle(const char* subCommand, const char* argument, void(*outputFunc)(const char* message)) = 0;
protected:
	virtual void toString(char* buffer) = 0;
	void done(const char* argument, void(*outputFunc)(const char* message));
	void error(const char* errorMessage, void(*outputFunc)(const char* message));
	bool preValidate(const char* argument, void(*outputFunc)(const char* message));
	void beforeSet(const char* argument, void(*outputFunc)(const char* message));
	void afterSet(const char* argument, void(*outputFunc)(const char* message));
};

void svm_initializer_hidden(SettingValueManager* svm);

#endif