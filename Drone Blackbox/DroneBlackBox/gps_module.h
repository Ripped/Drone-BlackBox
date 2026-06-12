#ifndef GPS_MODULE_H
#define GPS_MODULE_H

void initGPS();
void readGPS();

extern double lat, lng;
extern float speed;
extern int satelliteCount;
extern float hdopValue;

#endif