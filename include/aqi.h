#pragma once
#include <Arduino.h>

extern const int BP_PM25[7];
extern const int AQI_PM25[7];

extern const int BP_PM10[7];
extern const int AQI_PM10[7];

int calculateAQI(float C, const int BP[], const int AQI_BP[], int size);
