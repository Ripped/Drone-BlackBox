#include <WiFi.h>
#include "wifi_manager.h"

const char* ssid = "mojatv_full_2935";
const char* password = "IBFPJLBHGSIBFPJLBHGS";

void connectWiFi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}