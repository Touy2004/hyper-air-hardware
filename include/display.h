#pragma once
#include <Arduino.h>
#include "sensors.h"

void initDisplay();
void updateDisplay(const SensorData& data);   // non-blocking
