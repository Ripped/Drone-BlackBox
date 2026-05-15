from pydantic import BaseModel

class IMUData(BaseModel):
    ax: float
    ay: float
    az: float
    gx: float
    gy: float
    gz: float

class GPSData(BaseModel):
    lat: float
    lng: float
    speed: float

class FlightDataCreate(BaseModel):
    device_id: str
    timestamp: int
    imu: IMUData
    gps: GPSData