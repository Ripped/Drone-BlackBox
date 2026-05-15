#include <SPI.h>
#include <SD.h>
#include "storage.h"
#include "sensors.h"
#include "gps_module.h"

#define SD_CS 5
File logFile;

void initSD() {
  if (!SD.begin(SD_CS)) {
    Serial.println("SD init failed!");
    return;
  }
  
  logFile = SD.open("/log.csv", FILE_APPEND);
  if (logFile) {
    if (logFile.size() == 0) {
      logFile.println("ax,ay,az,gx,gy,gz,lat,lng,speed");
    }
    Serial.println("SD File Ready");
  }
}

void logToSD() {
  if (logFile) {
    logFile.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.6f,%.6f,%.2f\n", 
                   ax, ay, az, gx, gy, gz, lat, lng, speed);
    
    static int flushCounter = 0;
    if (++flushCounter > 10) { 
      logFile.flush(); 
      flushCounter = 0; 
    }
  }
}