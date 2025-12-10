#include "sensors.h"
#include "config.h"
#include "aqi.h"

#include <DHT.h>
#include <HardwareSerial.h>

// DHT
DHT dht(DHT_PIN, DHT_TYPE);

// PMS5003
HardwareSerial pmsSerial(1);

// Global sensor data
SensorData sensorData;

void initSensors() {
  dht.begin();
  pmsSerial.begin(9600, SERIAL_8N1, PIN_PMS_RX, PIN_PMS_TX);
}

// Non-blocking sensor update
void updateSensors() {
  // ---- PMS5003: read frames when available ----
  while (pmsSerial.available() >= 32) {
    uint8_t buf[32];
    pmsSerial.readBytes(buf, 32);
    if (buf[0] == 0x42 && buf[1] == 0x4D) {
      sensorData.pm25 = (buf[12] << 8) | buf[13];
      sensorData.pm10 = (buf[14] << 8) | buf[15];
    }
  }

  // ---- DHT + AQI only every SENSOR_INTERVAL_MS ----
  static uint32_t lastUpdate = 0;
  uint32_t now = millis();
  if (now - lastUpdate < SENSOR_INTERVAL_MS) return;
  lastUpdate = now;

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) sensorData.temp = t;
  if (!isnan(h)) sensorData.humi = h;

  if (sensorData.pm25 < 0) sensorData.pm25 = 0;
  if (sensorData.pm10 < 0) sensorData.pm10 = 0;

  sensorData.aqi25 = calculateAQI(sensorData.pm25, BP_PM25, AQI_PM25, 7);
  sensorData.aqi10 = calculateAQI(sensorData.pm10, BP_PM10, AQI_PM10, 7);

  Serial.printf("PM2.5=%d AQI2.5=%d | PM10=%d AQI10=%d\n",
                sensorData.pm25, sensorData.aqi25,
                sensorData.pm10, sensorData.aqi10);
  Serial.printf("Temp=%.1fC Humi=%.1f%%\n", sensorData.temp, sensorData.humi);
}
