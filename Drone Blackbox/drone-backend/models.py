from sqlalchemy import Column, Integer, Float, String, ForeignKey, DateTime
from sqlalchemy.orm import relationship
from database import Base
import datetime

class FlightSession(Base):
    __tablename__ = "flight_sessions"
    id = Column(Integer, primary_key=True, index=True)
    device_id = Column(String)
    start_time = Column(DateTime, default=datetime.datetime.utcnow)
    
    data_points = relationship("FlightData", back_populates="session")

class FlightData(Base):
    __tablename__ = "flight_data"

    id = Column(Integer, primary_key=True, index=True)
    flight_id = Column(Integer, ForeignKey("flight_sessions.id"))
    timestamp = Column(Integer)

    ax = Column(Float); ay = Column(Float); az = Column(Float)
    gx = Column(Float); gy = Column(Float); gz = Column(Float)

    lat = Column(Float)
    lng = Column(Float)
    speed = Column(Float)

    roll = Column(Float)
    pitch = Column(Float)
    is_free_fall = Column(Integer)

    session = relationship("FlightSession", back_populates="data_points")