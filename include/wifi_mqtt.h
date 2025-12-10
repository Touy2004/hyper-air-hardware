#pragma once
#include <Arduino.h>
#include <PubSubClient.h>
#include "sensors.h"

extern PubSubClient mqttClient;

void setupWiFiAndMQTT();
void ensureMQTTConnected();
void publishSensorData(const SensorData& data);
