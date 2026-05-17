#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "sensors.h"

Adafruit_MPU6050 mpu;
float ax, ay, az, gx, gy, gz;
float roll = 0, pitch = 0;
float off_ax = 0, off_ay = 0, off_az = 0, off_gx = 0, off_gy = 0, off_gz = 0;

unsigned long prevTime = 0;
bool sensorAvailable = false;

void initSensors() {
    Wire.begin(21, 22); 

    if (!mpu.begin()) {
        Serial.println("MPU6050 nije pronadjen!");
        sensorAvailable = false;
        return;
    }
    
    sensorAvailable = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    calibrateMPU();
    prevTime = millis();
}

void calibrateMPU() {
    if (!sensorAvailable) return;

    Serial.println("Kalibracija... NE POMICITE DRON!");
    float sum_ax=0, sum_ay=0, sum_az=0, sum_gx=0, sum_gy=0, sum_gz=0;
    int samples = 200;
    
    for(int i=0; i<samples; i++) {
        sensors_event_t a, g, temp;
        mpu.getEvent(&a, &g, &temp);
        sum_ax += a.acceleration.x; sum_ay += a.acceleration.y; sum_az += a.acceleration.z;
        sum_gx += g.gyro.x; sum_gy += g.gyro.y; sum_gz += g.gyro.z;
        delay(2);
    }
    off_ax = sum_ax/samples; 
    off_ay = sum_ay/samples; 
    off_az = (sum_az/samples) - 9.81;
    off_gx = sum_gx/samples; 
    off_gy = sum_gy/samples; 
    off_gz = sum_gz/samples;
    Serial.println("Kalibracija gotova.");
}

void readMPU() {
    if (!sensorAvailable) {
        ax = ay = az = gx = gy = gz = roll = pitch = 0;
        return;
    }

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    ax = a.acceleration.x - off_ax;
    ay = a.acceleration.y - off_ay;
    az = a.acceleration.z - off_az;
    gx = (g.gyro.x - off_gx) * 57.2958;
    gy = (g.gyro.y - off_gy) * 57.2958;
    gz = (g.gyro.z - off_gz) * 57.2958;

    float dt = (millis() - prevTime) / 1000.0;
    prevTime = millis();

    if (dt <= 0) dt = 0.01; 

    float rollAcc = atan2(ay, az) * 57.2958;
    float pitchAcc = atan2(-ax, sqrt(ay*ay + az*az)) * 57.2958;

    roll = 0.98 * (roll + gx * dt) + 0.02 * rollAcc;
    pitch = 0.98 * (pitch + gy * dt) + 0.02 * pitchAcc;
}