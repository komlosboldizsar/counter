#ifndef SERIAL_H_
#define SERIAL_H_

#include <Arduino.h>

void serialInit();
void serialRead();
void serialHandleCommand();

#endif