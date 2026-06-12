#include <SPI.h>
#include <SD.h>
#include "storage.h"
#include "sensors.h"
#include "gps_module.h"

#define SD_CS 5

File logFile;
String currentFlightFile = "";
int globalFlightNumber = 1;

static int flushCounter = 0;

void enforceCircularBuffer() {
    uint64_t total = SD.totalBytes();
    uint64_t used = SD.usedBytes();
    uint64_t freeMB = (total - used) / (1024 * 1024);

    if (freeMB < 50) {
        Serial.println("[SD] LOW SPACE CLEANUP");

        int i = 1;
        while (freeMB < 50 && i < globalFlightNumber) {
            String f = "/let" + String(i) + ".csv";

            if (SD.exists(f)) {
                SD.remove(f);
                Serial.println("[SD] deleted " + f);
            }

            used = SD.usedBytes();
            freeMB = (total - used) / (1024 * 1024);
            i++;
        }
    }
}

void initSD() {
    Serial.println("[SD] init...");

    if (!SD.begin(SD_CS)) {
        Serial.println("[SD] FAILED");
        return;
    }

    enforceCircularBuffer();

    int flight = 1;
    while (true) {
        String f = "/let" + String(flight) + ".csv";
        if (!SD.exists(f)) {
            currentFlightFile = f;
            globalFlightNumber = flight;
            break;
        }
        flight++;
    }

    Serial.println("[SD] file: " + currentFlightFile);

    logFile = SD.open(currentFlightFile, FILE_WRITE);

    if (!logFile) {
        Serial.println("[SD] file open failed");
        return;
    }

    logFile.println("ax,ay,az,gx,gy,gz,lat,lng,speed");
    logFile.flush();
}

void logToSD() {
    if (!logFile) return;

    logFile.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.6f,%.6f,%.2f\n",
                   ax, ay, az,
                   gx, gy, gz,
                   lat, lng,
                   speed);

    if (++flushCounter >= 5) {
        logFile.flush();
        flushCounter = 0;
    }
}

void closeSD() {
    if (logFile) {
        logFile.flush();
        logFile.close();
        Serial.println("[SD] safely closed");
    }
}