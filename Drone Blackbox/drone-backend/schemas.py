from pydantic import BaseModel
from typing import Optional


class IMUData(BaseModel):
    ax: float
    ay: float
    az: float
    gx: float
    gy: float
    gz: float


class GPSData(BaseModel):
    lat: Optional[float] = None
    lng: Optional[float] = None
    speed: Optional[float] = None


class FlightDataCreate(BaseModel):
    device_id: str
    timestamp: Optional[int] = None
    imu: IMUData
    gps: GPSData