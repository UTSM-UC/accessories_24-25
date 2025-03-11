#include "SystemUtils.h"

SystemUtils::SystemUtils() {
    previousMillis = 0;
}

void SystemUtils::heartbeat() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= HEARTBEAT_INTERVAL) {
        previousMillis = currentMillis;

        if (digitalRead(LED_PIN) == LOW) {
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
    }
}