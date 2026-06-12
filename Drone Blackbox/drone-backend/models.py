from sqlalchemy import Column, Integer, Float, String, ForeignKey, DateTime, Boolean
from sqlalchemy.orm import relationship
from database import Base
import datetime

class FlightSession(Base):
    __tablename__ = "flight_sessions"

    id = Column(Integer, primary_key=True, index=True)
    device_id = Column(String, nullable=False)

    start_time = Column(
        DateTime,
        default=lambda: datetime.datetime.now(datetime.timezone.utc)
    )

    data_points = relationship(
        "FlightData",
        back_populates="session",
        cascade="all, delete-orphan"
    )


class FlightData(Base):
    __tablename__ = "flight_data"

    id = Column(Integer, primary_key=True, index=True)

    flight_id = Column(Integer, ForeignKey("flight_sessions.id"), index=True, nullable=False)

    timestamp = Column(DateTime, default=datetime.datetime.utcnow, index=True)

    ax = Column(Float)
    ay = Column(Float)
    az = Column(Float)

    gx = Column(Float)
    gy = Column(Float)
    gz = Column(Float)

    lat = Column(Float)
    lng = Column(Float)
    speed = Column(Float)

    roll = Column(Float)
    pitch = Column(Float)

    is_free_fall = Column(Boolean, default=False)
    is_crash = Column(Boolean, default=False)

    session = relationship("FlightSession", back_populates="data_points")