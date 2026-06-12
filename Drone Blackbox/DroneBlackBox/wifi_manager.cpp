#include <WiFi.h>
#include "wifi_manager.h"

/*const char* ssid = "23F870";
const char* password = "EVW32C0N00034190";*/

const char* ssid = "mojatv_full_2935";
const char* password = "IBFPJLBHGSIBFPJLBHGS";

void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("[WIFI] Connecting...");

  WiFi.disconnect(true);
  delay(300);

  WiFi.begin(ssid, password);

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(200);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] OK");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[WIFI] FAIL");
  }
}