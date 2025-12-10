#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiManager.h>   // <— NEW
#include <PubSubClient.h>

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- DHT22 ----------------
#define DHTPIN 18
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------------- PMS5003 ----------------
HardwareSerial pmsSerial(1);  // RX=16, TX=17

// ---------------- IR SENSOR ----------------
const int irPin = 2;  // Black wire

// ---------------- MOTOR DRIVER ----------------
const int in1 = 26;
const int in2 = 27;

// ---------------- MQTT ----------------
// WiFiManager will handle WiFi SSID & password - no need to hardcode here

const char* mqtt_server = "183.182.103.136";
const int mqtt_port = 1883;
const char* mqtt_user = "mrc";
const char* mqtt_pass = "123456";

WiFiClient espClient;
PubSubClient client(espClient);

// WiFiManager object
WiFiManager wm;

// ---------------- AQI breakpoints ----------------
const int BP_PM25[7]  = { 0, 12, 35, 55, 150, 250, 350 };
const int AQI_PM25[7] = { 0, 50, 100, 150, 200, 300, 400 };

const int BP_PM10[7]  = { 0, 54, 154, 254, 354, 424, 504 };
const int AQI_PM10[7] = { 0, 50, 100, 150, 200, 300, 400 };

// ---------------- AQI calculation ----------------
int calculateAQI(float C, const int BP[], const int AQI_BP[], int size) {
  for (int i = 1; i < size; i++) {
    if (C <= BP[i]) {
      float Clow = BP[i - 1];
      float Chigh = BP[i];
      int Ilow = AQI_BP[i - 1];
      int Ihigh = AQI_BP[i];
      return round((Ihigh - Ilow) * (C - Clow) / (Chigh - Clow) + Ilow);
    }
  }
  return AQI_BP[size - 1];  // max AQI
}

// ---------------- Timing ----------------
unsigned long lastUpdate = 0;
const long interval = 2000;  // 2s

int pm25 = -1;
int pm10 = -1;
float temp = 0;
float humi = 0;

// ---------------- MQTT reconnect ----------------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("Connected to MQTT");
      // subscribe topics here if needed
      // client.subscribe("your/topic");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" - retrying in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));    // for MQTT clientId randomness

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  dht.begin();
  pmsSerial.begin(9600, SERIAL_8N1, 16, 17);
  pinMode(irPin, INPUT_PULLUP);  // IR sensor

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();

  // ---------------- WiFiManager: handle WiFi ----------------
  WiFi.mode(WIFI_STA);  // station mode

  // This starts config portal if it can't connect to saved WiFi
  // AP Name: "HyperAir-Setup", Password: "12345678"
  bool res = wm.autoConnect("HyperAir-Setup", "12345678");

  if (!res) {
    Serial.println("Failed to connect via WiFiManager");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi FAILED");
    // You can choose to reset:
    // ESP.restart();
  } else {
    Serial.println("WiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK ");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
    delay(1500);
    lcd.clear();
  }

  // MQTT setup
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // --------- Keep WiFi alive with WiFiManager (optional recheck) ---------
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost! Re-open portal...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Lost");
    lcd.setCursor(0, 1);
    lcd.print("Reconfig...");

    // block here until user reconfigures WiFi
    bool res = wm.autoConnect("HyperAir-Setup", "12345678");
    if (res) {
      Serial.println("WiFi reconnected!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi OK");
      delay(1000);
      lcd.clear();
    } else {
      Serial.println("Still no WiFi...");
      delay(2000);
    }
  }

  // --------- MQTT keep-alive ---------
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // -------- Read PMS5003 continuously --------
  while (pmsSerial.available() >= 32) {
    uint8_t buf[32];
    pmsSerial.readBytes(buf, 32);
    if (buf[0] == 0x42 && buf[1] == 0x4D) {
      pm25 = (buf[12] << 8) | buf[13];
      pm10 = (buf[14] << 8) | buf[15];
    }
  }

  // -------- Read IR sensor --------
  if (digitalRead(irPin) == LOW) {
    Serial.println("Object Detected!");
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    delay(4000);
  } else {
    Serial.println("No object.");
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  // -------- Update every interval --------
  if (millis() - lastUpdate >= interval) {
    lastUpdate = millis();

    // Read DHT22
    temp = dht.readTemperature();
    humi = dht.readHumidity();
    if (isnan(temp)) temp = 0;
    if (isnan(humi)) humi = 0;

    // Calculate AQI
    int AQI2_5 = calculateAQI(pm25, BP_PM25, AQI_PM25, 7);
    int AQI10  = calculateAQI(pm10, BP_PM10, AQI_PM10, 7);

    // Serial debug
    Serial.printf("PM2.5=%dµg/m3 AQI2.5=%d | PM10=%dµg/m3 AQI10=%d\n", pm25, AQI2_5, pm10, AQI10);
    Serial.printf("Temp=%.1fC Humi=%.1f%%\n", temp, humi);

    // Publish MQTT
    char msg[150];
    snprintf(msg, sizeof(msg),
             "{\"AQI2_5\":%d,\"AQI10\":%d,\"Temp\":%.1f,\"Humi\":%.1f}",
             AQI2_5, AQI10, temp, humi);
    client.publish("mrc/hyperair/data", msg);
    Serial.println(msg);

    // Display PM / AQI
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("PM2.5=%d", AQI2_5);
    lcd.print((char)230);
    lcd.print("g/m3");

    lcd.setCursor(0, 1);
    lcd.printf("PM10=%d", AQI10);
    lcd.print((char)230);
    lcd.print("g/m3");
    delay(2000);

    // Display Temp / Humi
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.printf("Temp:%.1f", temp);
    lcd.write(223);
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.printf("Humi:%.1f%%", humi);
    delay(2000);
  }
}
