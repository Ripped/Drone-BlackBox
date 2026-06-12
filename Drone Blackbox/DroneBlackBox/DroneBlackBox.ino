#include "sensors.h"
#include "gps_module.h"
#include "wifi_manager.h"
#include "storage.h"
#include "api.h"
#include <WiFi.h>

#define LED_PIN 2

unsigned long lastRead = 0;

extern bool serverAvailable;

void handleStatusLED() {
    static unsigned long lastToggle = 0;

    if (WiFi.status() == WL_CONNECTED && serverAvailable && flight_id != -1) {
        digitalWrite(LED_PIN, HIGH);
        return;
    }

    if (millis() - lastToggle > 500) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastToggle = millis();
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);

    initSensors();
    initGPS();
    initSD();

    connectWiFi();
    initAPI();

    Serial.println("SYSTEM READY");
}

void loop() {
    unsigned long now = millis();

    handleStatusLED();

    readGPS();

    if (now - lastRead >= 200) {
        lastRead = now;

        readMPU();
        logToSD();
    }

    static unsigned long printTimer = 0;
    if (now - printTimer > 1000) {
        printTimer = now;

        Serial.println("---- DATA ----");
        Serial.printf("ACC %.2f %.2f %.2f\n", ax, ay, az);
        Serial.printf("GYRO %.2f %.2f %.2f\n", gx, gy, gz);
        Serial.printf("LAT %.6f LNG %.6f\n", lat, lng);
        Serial.printf("SPD %.2f\n", speed);
        Serial.printf("SAT %d HDOP %.2f\n", satelliteCount, hdopValue);
        Serial.println("----------------");
    }
}