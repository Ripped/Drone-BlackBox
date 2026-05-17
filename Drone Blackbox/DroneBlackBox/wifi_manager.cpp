#include <WiFi.h>
#include "wifi_manager.h"

const char* ssid = "23F870";
const char* password = "EVW32C0N00034190";

void connectWiFi() {
  Serial.print("[WIFI] Povezivanje na mrezu...");
  WiFi.begin(ssid, password);

  int timeoutCounter = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeoutCounter++;
    
    if (timeoutCounter > 20) { 
      Serial.println("\n[WIFI] GRESKA: Povezivanje nije uspjelo! Sistem nastavlja rad offline (SD mod).");
      break;
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Uspjesno povezan! IP adresa drona: ");
    Serial.println(WiFi.localIP());
  }
}