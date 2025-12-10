#pragma once

#include <Arduino.h>
#include <DHT.h>   // for DHT11 / DHT22 constants

// ---------- WiFiManager AP (for config portal) ----------
constexpr char WIFI_AP_NAME[]     = "HyperAir-Setup";
constexpr char WIFI_AP_PASSWORD[] = "12345678";

// ---------- MQTT Config ----------
constexpr char MQTT_SERVER[]      = "183.182.103.136";  // TODO
constexpr uint16_t MQTT_PORT      = 1883;
constexpr char MQTT_USER[]        = "mrc";
constexpr char MQTT_PASS[]        = "123456";
constexpr char MQTT_TOPIC_DATA[]  = "mrc/hyperair/data";

// ---------- LCD ----------
constexpr uint8_t LCD_I2C_ADDR    = 0x27;

// ---------- Pins ----------
constexpr uint8_t DHT_PIN         = 18;
constexpr uint8_t DHT_TYPE        = DHT22;  // DHT.h macro

constexpr uint8_t PIN_IR          = 2;
constexpr uint8_t PIN_IN1         = 26;
constexpr uint8_t PIN_IN2         = 27;

constexpr uint8_t PIN_PMS_RX      = 16;
constexpr uint8_t PIN_PMS_TX      = 17;

// ---------- Timing (ms) ----------
constexpr uint32_t SENSOR_INTERVAL_MS   = 2000;  // DHT + AQI update
constexpr uint32_t LCD_PAGE_INTERVAL_MS = 2000;  // switch LCD page
constexpr uint32_t MOTOR_ON_MS          = 4000;  // motor run after detect
