#include <TinyGPS++.h>
#include "gps_module.h"

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

double lat = 0;
double lng = 0;
float speed = 0;

int satelliteCount = 0;
float hdopValue = 99.99;

double latSum = 0, lngSum = 0;
int sampleCount = 0;

void initGPS() {
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
}

void readGPS() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  satelliteCount = gps.satellites.isValid() ? gps.satellites.value() : 0;
  hdopValue = gps.hdop.isValid() ? gps.hdop.hdop() : 99.99;

  if (gps.location.isValid()) {
    speed = gps.speed.kmph();

    if (hdopValue < 2.0) {
      latSum += gps.location.lat();
      lngSum += gps.location.lng();
      sampleCount++;

      if (sampleCount >= 5) {
        lat = latSum / 5.0;
        lng = lngSum / 5.0;

        latSum = 0;
        lngSum = 0;
        sampleCount = 0;
      }
    }
  }
}