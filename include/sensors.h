#pragma once
#include <Arduino.h>

struct SensorData {
  int   pm25  = -1;
  int   pm10  = -1;
  float temp  = NAN;
  float humi  = NAN;
  int   aqi25 = 0;
  int   aqi10 = 0;
};

extern SensorData sensorData;

void initSensors();
void updateSensors();   // non-blocking, call every loop()
