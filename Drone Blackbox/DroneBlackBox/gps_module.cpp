#include <TinyGPS++.h>
#include "gps_module.h"

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

double lat = 0;
double lng = 0;
float speed = 0;
int satelliteCount = 0;

double latSum = 0, lngSum = 0;
int sampleCount = 0;

void initGPS() {
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
}

void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  satelliteCount = gps.satellites.value();

  if (gps.location.isValid()) {
    float currentSpeed = gps.speed.kmph();
    
    float hdop = gps.hdop.value() / 100.0; 
    speed = currentSpeed;

    if (hdop < 2.0) { 
      if (currentSpeed > 1.2 || lat == 0) {
        
        latSum += gps.location.lat();
        lngSum += gps.location.lng();
        sampleCount++;

        if (sampleCount >= 5) {
          lat = latSum / 5;
          lng = lngSum / 5;
          
          latSum = 0; 
          lngSum = 0; 
          sampleCount = 0;
        }
      }
    }
  }
}