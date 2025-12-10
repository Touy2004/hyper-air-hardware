#include "display.h"
#include "config.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);

void initDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
  lcd.clear();
}

// Switch between 2 pages every LCD_PAGE_INTERVAL_MS
void updateDisplay(const SensorData& data) {
  static uint32_t lastSwitch = 0;
  static uint8_t page = 0;

  uint32_t now = millis();
  if (now - lastSwitch < LCD_PAGE_INTERVAL_MS) return;
  lastSwitch = now;
  page = (page + 1) % 2;

  lcd.clear();

  if (page == 0) {
    // Page 1: AQI
    lcd.setCursor(0, 0);
    lcd.print("PM2.5: ");
    lcd.print(data.aqi25);

    lcd.setCursor(0, 1);
    lcd.print("PM10: ");
    lcd.print(data.aqi10);
  } else {
    // Page 2: Temp / Humidity
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(data.temp, 1);
    lcd.write(223); // degree symbol
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Humi: ");
    lcd.print(data.humi, 1);
    lcd.print("%");
  }
}
