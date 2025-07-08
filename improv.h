#ifndef IMPROV_H_
#define IMPROV_H_

#include <Arduino.h>

void improvInit();
void improvMainLoop(unsigned long now, bool* wifiChanged);

#endif