#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include "api.h"
#include "sensors.h"
#include "gps_module.h"

int flight_id = -1; 

void getFlightId() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        //http.begin("http://192.168.1.28:8000/api/session/start?device_id=drone_01");
        http.begin("http://192.168.0.15:8000/api/session/start?device_id=drone_01");

        int httpResponseCode = http.POST(""); 
        
        if (httpResponseCode == 200) {
            String payload = http.getString();
            StaticJsonDocument<64> doc;
            deserializeJson(doc, payload);
            flight_id = doc["flight_id"];
            
            Serial.print("[API] Nova sesija otvorena. Flight ID: ");
            Serial.println(flight_id);
        } else {
            Serial.print("[API] Greška pri otvaranju sesije: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    }
}

void uploadTaskCode(void * pvParameters) {
    for(;;) {
        if (WiFi.status() == WL_CONNECTED && flight_id == -1) {
            getFlightId();
        }

        // SLANJE PODATAKA
        if (WiFi.status() == WL_CONNECTED && flight_id != -1) {
            HTTPClient http;
            //String url = "http://192.168.1.28:8000/api/data/" + String(flight_id);
            String url = "http://192.168.0.15:8000/api/data/" + String(flight_id);
            http.begin(url);
            http.addHeader("Content-Type", "application/json");

            String json = "{";
            json += "\"device_id\":\"drone_01\",";
            json += "\"timestamp\":" + String(millis()) + ",";
            json += "\"imu\":{";
            json += "\"ax\":" + String(ax) + ",\"ay\":" + String(ay) + ",\"az\":" + String(az) + ",";
            json += "\"gx\":" + String(gx) + ",\"gy\":" + String(gy) + ",\"gz\":" + String(gz);
            json += "},";
            json += "\"gps\":{";
            json += "\"lat\":" + String(lat, 6) + ",\"lng\":" + String(lng, 6) + ",\"speed\":" + String(speed);
            json += "}";
            json += "}";

            int httpResponseCode = http.POST(json);
            
            if (httpResponseCode > 0) {
                Serial.printf("[API] Podaci poslani. Status: %d\n", httpResponseCode);
            } else {
                Serial.printf("[API] Slanje neuspješno: %s\n", http.errorToString(httpResponseCode).c_str());
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