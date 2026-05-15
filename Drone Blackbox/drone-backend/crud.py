import math
import models

def create_flight_data(db, data, flight_id):
    roll = math.atan2(data.imu.ay, data.imu.az) * 57.2958
    pitch = math.atan2(-data.imu.ax, math.sqrt(data.imu.ay**2 + data.imu.az**2)) * 57.2958

    total_acc = math.sqrt(data.imu.ax**2 + data.imu.ay**2 + data.imu.az**2) / 9.81
    free_fall = 1 if total_acc < 0.3 else 0 

    db_data = models.FlightData(
        flight_id=flight_id,
        timestamp=data.timestamp,
        ax=data.imu.ax, ay=data.imu.ay, az=data.imu.az,
        gx=data.imu.gx, gy=data.imu.gy, gz=data.imu.gz,
        
        lat=data.gps.lat, 
        lng=data.gps.lng, 
        speed=data.gps.speed,
        
        roll=round(roll, 2),
        pitch=round(pitch, 2),
        is_free_fall=free_fall
    )
    db.add(db_data)
    db.commit()
    return db_data

def start_new_session(db, device_id: str):
    new_session = models.FlightSession(device_id=device_id)
    db.add(new_session)
    db.commit()
    db.refresh(new_session)
    return new_session