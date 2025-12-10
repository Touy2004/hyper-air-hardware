#include "motor_ir.h"
#include "config.h"

void initMotorAndIR() {
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);

  pinMode(PIN_IR, INPUT_PULLUP);
}

// IR low → start motor for MOTOR_ON_MS (no delay)
void updateMotorAndIR() {
  static bool motorOn = false;
  static uint32_t motorOnStart = 0;

  uint32_t now = millis();
  int irState = digitalRead(PIN_IR);

  if (irState == LOW && !motorOn) {
    motorOn = true;
    motorOnStart = now;

    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, HIGH);
    Serial.println("Object Detected! Motor ON");
  }

  if (motorOn && (now - motorOnStart >= MOTOR_ON_MS)) {
    motorOn = false;
    digitalWrite(PIN_IN1, LOW);
    digitalWrite(PIN_IN2, LOW);
    Serial.println("Motor OFF");
  }
}
