#include <Arduino.h>

#include "config.h"
#include "sensors.h"
#include "display.h"
#include "motor_ir.h"
#include "wifi_mqtt.h"

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("==== HyperAir ESP32 Start ====");

  initDisplay();
  initSensors();
  initMotorAndIR();
  setupWiFiAndMQTT();
}

void loop() {
  // Always keep sensors and IO updated
  updateSensors();
  updateMotorAndIR();

  // Network
  ensureMQTTConnected();
  mqttClient.loop();  // keep MQTT alive

  // Publish periodically (same rate as SENSOR_INTERVAL_MS)
  static uint32_t lastPublish = 0;
  uint32_t now = millis();
  if (now - lastPublish >= SENSOR_INTERVAL_MS) {
    lastPublish = now;
    publishSensorData(sensorData);
  }

  // Update LCD pages (AQI ↔ Temp/Humi)
  updateDisplay(sensorData);
}
