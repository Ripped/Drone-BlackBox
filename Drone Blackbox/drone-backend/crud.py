import math
import datetime
import models
from sqlalchemy.exc import SQLAlchemyError


def create_flight_data(db, data, flight_id):

    imu = data.imu
    gps = data.gps if hasattr(data, "gps") else None

    roll = math.atan2(imu.ay, imu.az) * 57.2958
    pitch = math.atan2(-imu.ax, math.sqrt(imu.ay**2 + imu.az**2)) * 57.2958

    total_g_force = math.sqrt(imu.ax**2 + imu.ay**2 + imu.az**2) / 9.81

    free_fall = total_g_force < 0.4
    crash_impact = total_g_force > 2.5

    db_data = models.FlightData(
        flight_id=int(flight_id),

        timestamp=datetime.datetime.utcnow(),

        ax=imu.ax,
        ay=imu.ay,
        az=imu.az,

        gx=imu.gx,
        gy=imu.gy,
        gz=imu.gz,

        lat=gps.lat if gps else None,
        lng=gps.lng if gps else None,
        speed=gps.speed if gps else None,

        roll=round(roll, 2),
        pitch=round(pitch, 2),

        is_free_fall=free_fall,
        is_crash=crash_impact
    )

    try:
        db.add(db_data)
        db.commit()
        return db_data

    except SQLAlchemyError as e:
        db.rollback()
        print(f"[DATABASE ERROR] {str(e)}")
        return None


def start_new_session(db, device_id: str):
    new_session = models.FlightSession(device_id=device_id)
    db.add(new_session)
    db.commit()
    db.refresh(new_session)
    return new_session