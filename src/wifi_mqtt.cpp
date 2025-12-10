#include "wifi_mqtt.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiManager.h>

WiFiManager wm;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Internal helper
static void connectWiFiIfNeeded() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("WiFi not connected, starting WiFiManager portal...");
  bool res = wm.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD);

  if (!res) {
    Serial.println("WiFiManager failed to connect");
  } else {
    Serial.print("WiFi connected, IP: ");
    Serial.println(WiFi.localIP());
  }
}

void setupWiFiAndMQTT() {
  WiFi.mode(WIFI_STA);
  connectWiFiIfNeeded();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
}

void ensureMQTTConnected() {
  connectWiFiIfNeeded();
  if (WiFi.status() != WL_CONNECTED) return;

  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
      // mqttClient.subscribe("your/topic/here");  // if needed
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);   // OK here, only on failure
    }
  }
}

void publishSensorData(const SensorData& data) {
  if (!mqttClient.connected()) return;

  char msg[160];
  snprintf(msg, sizeof(msg),
           "{\"AQI2_5\":%d,\"AQI10\":%d,\"Temp\":%.1f,\"Humi\":%.1f}",
           data.aqi25, data.aqi10, data.temp, data.humi);

  bool ok = mqttClient.publish(MQTT_TOPIC_DATA, msg);

  if (ok) {
    Serial.print("MQTT publish: ");
    Serial.println(msg);
  } else {
    Serial.println("MQTT publish failed");
  }
}
