#include "sensors.h"
#include "gps_module.h"
#include "wifi_manager.h"
#include <WiFi.h>
#include "storage.h"
#include "api.h"

#define LED_PIN 2
unsigned long lastRead = 0;

void handleStatusLED() {
    static unsigned long lastToggle = 0;
    int interval = 1000;

    if (WiFi.status() == WL_CONNECTED && flight_id != -1) {
        digitalWrite(LED_PIN, HIGH);
        return;
    } else if (WiFi.status() == WL_CONNECTED) {
        interval = 200;
    }

    if (millis() - lastToggle >= interval) {
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        lastToggle = millis();
    }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);

  pinMode(2, OUTPUT);
  
  connectWiFi(); 
  delay(500);
  
  initSensors(); 
  initGPS();
  initSD();
  
  initAPI();

  Serial.println("System Ready");
}

void loop() {
  unsigned long currentMillis = millis();
  handleStatusLED();

  if (currentMillis - lastRead >= 200) {
    lastRead = currentMillis;
    readMPU();
    readGPS();
    logToSD();
    printData();
  }

}
void printData() {
  Serial.println("---- DATA ----");

  Serial.printf("ACC: %.2f %.2f %.2f\n", ax, ay, az);
  Serial.printf("GYRO: %.2f %.2f %.2f\n", gx, gy, gz);

  Serial.printf("GPS: %.6f %.6f\n", lat, lng);
  Serial.printf("Speed: %.2f km/h\n", speed);

  Serial.println("----------------\n");
}