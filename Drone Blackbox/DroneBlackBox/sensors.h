#ifndef SENSORS_H
#define SENSORS_H

void initSensors();
void readMPU();
void calibrateMPU(); 

extern float ax, ay, az;
extern float gx, gy, gz;
extern float roll, pitch;

#endif