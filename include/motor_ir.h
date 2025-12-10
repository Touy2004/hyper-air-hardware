#pragma once
#include <Arduino.h>

void initMotorAndIR();
void updateMotorAndIR();  // non-blocking, call every loop()
