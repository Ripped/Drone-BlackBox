#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "storage.h" 
#include "api.h"
#include "sensors.h"
#include "gps_module.h"

int flight_id = -1; 
bool serverAvailable = true;

void getFlightId() {
    if (WiFi.status() == WL_CONNECTED) { 
        HTTPClient http;
        http.begin("http://192.168.1.19:8000/api/session/start?device_id=drone_01");
        http.setTimeout(3000); 

        int httpResponseCode = http.POST(""); 
        
        if (httpResponseCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<128> doc;
            deserializeJson(doc, payload);
            flight_id = doc["flight_id"]; 
            serverAvailable = true;
            
            Serial.print("[API] Nova sesija otvorena. Flight ID: ");
            Serial.println(flight_id);

            if (doc.containsKey("date")) {
                String serverDate = doc["date"].as<String>();
                String newFileName = "/let_" + String(flight_id) + "_" + serverDate + ".csv";

                if (SD.rename(currentFlightFile, newFileName)) {
                    Serial.print("[SD] Fajl uspjesno sinhronizovan i preimenovan u: ");
                    Serial.println(newFileName);
                    currentFlightFile = newFileName;
                }
            }
        } else {
            Serial.print("[API] Neuspjesno otvaranje sesije. Status: ");
            Serial.println(httpResponseCode);
            serverAvailable = false;
        }
        http.end();
    }
}

void processHardwareCommands() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://192.168.1");
        http.setTimeout(2000);
        
        int httpCode = http.GET();
        if (httpCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<96> doc;
            deserializeJson(doc, payload);
            
            int deleteTargetId = doc["delete_flight_id"];
            bool shouldClearAll = doc["clear_all"];

            if (shouldClearAll) {
                Serial.println("[SD] !!! PRIMLJENA KOMANDA ZA POTPUNO CISCENJE KARTICE !!!");
                File root = SD.open("/");
                if (root) {
                    File file = root.openNextFile();
                    while (file) {
                        String fileName = String(file.name());
                        file.close();
                        if (fileName.endsWith(".csv")) {
                            SD.remove("/" + fileName);
                        }
                        file = root.openNextFile();
                    }
                    root.close();
                }
                
                HTTPClient httpConfirm;

                httpConfirm.begin("http://192.168.1");
                httpConfirm.POST("");
                httpConfirm.end();
                
                Serial.println("[SD] Kompletna SD kartica ociscena. Potvrda poslana.");
                Serial.println("[SISTEM] Pokrecem automatsku re-inicijalizaciju novog leta...");
                
                flight_id = -1;
                serverAvailable = true;
                initSD(); 
                return; 
            }

            if (deleteTargetId != -1) {
                if (deleteTargetId == flight_id) {
                    Serial.println("[SD] ODBIJENO BRISANJE: Let je trenutno aktivan i snima se!");
                    HTTPClient httpConfirm;
                    httpConfirm.begin("http://192.168.1");
                    httpConfirm.POST("");
                    httpConfirm.end();
                    return;
                }

                Serial.printf("[SD] Primljena komanda za brisanje Leta #%d...\n", deleteTargetId);
                File root = SD.open("/");
                if (root) {
                    File file = root.openNextFile();
                    while (file) {
                        String fileName = String(file.name());
                        file.close();
                        String targetPrefix = "let_" + String(deleteTargetId) + "_";
                        
                        if (fileName.startsWith(targetPrefix)) {
                            Serial.printf("[SD] Brisem pojedinacni fajl: /%s\n", fileName.c_str());
                            SD.remove("/" + fileName);
                            break;
                        }
                        file = root.openNextFile();
                    }
                    root.close();
                }

                HTTPClient httpConfirm;
                httpConfirm.begin("http://192.168.1");
                httpConfirm.POST("");
                httpConfirm.end();
                Serial.println("[SD] Potvrda o brisanju pojedinacnog fajla poslana.");
            }
        }
        http.end();
    }
}

void uploadTaskCode(void * pvParameters) {
    for(;;) {
        
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[API] WiFi mreza nedostupna. Pokusavam asinhrono povezivanje u pozadini...");
            WiFi.begin("mojatv_full_2935", "IBFPJLBHGSIBFPJLBHGS");
            serverAvailable = false;
        } 

        else if (WiFi.status() == WL_CONNECTED && flight_id == -1) {
            Serial.println("[API] WiFi dostupan! Pokusavam dohvatiti novi Flight ID sa servera...");
            getFlightId();
        }


        processHardwareCommands();


        if (WiFi.status() == WL_CONNECTED && flight_id != -1) {
            HTTPClient http;
            String url = "http://192.168.1.19:8000/api/data/" + String(flight_id);
            http.begin(url);
            http.addHeader("Content-Type", "application/json");
            http.setTimeout(3000);

            StaticJsonDocument<300> doc;
            doc["device_id"] = "drone_01";
            doc["timestamp"] = millis();

            JsonObject imuObj = doc.createNestedObject("imu");
            imuObj["ax"] = ax; imuObj["ay"] = ay; imuObj["az"] = az;
            imuObj["gx"] = gx; imuObj["gy"] = gy; imuObj["gz"] = gz;

            JsonObject gpsObj = doc.createNestedObject("gps");
            gpsObj["lat"] = lat; gpsObj["lng"] = lng; gpsObj["speed"] = speed;

            String json;
            serializeJson(doc, json);

            int httpResponseCode = http.POST(json); 
            
            if (httpResponseCode > 0) {
                if (!serverAvailable) {
                    Serial.println("[API] Veza uspostavljena! Nastavljam slanje na postojecu sesiju.");
                }
                Serial.printf("[API] Podaci poslani. Status: %d\n", httpResponseCode);
                serverAvailable = true;
            } else {
                Serial.printf("[API] Slanje neuspjesno: %s\n", http.errorToString(httpResponseCode).c_str());
                serverAvailable = false; 
            }
            http.end();
        }
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void initAPI() {
    
    getFlightId(); 

    xTaskCreatePinnedToCore(
        uploadTaskCode,   
        "UploadTask",     
        10000,            
        NULL,             
        1,                
        NULL,             
        0                 
    );
}