from fastapi import FastAPI, Depends, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session
import math 

import models, schemas, crud
from database import engine, SessionLocal

models.Base.metadata.create_all(bind=engine)

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

@app.post("/api/session/start")
def start_session(device_id: str, db: Session = Depends(get_db)):
    session = crud.start_new_session(db, device_id)
    return {"flight_id": session.id}

@app.post("/api/data/{flight_id}")
def receive_data(flight_id: int, data: schemas.FlightDataCreate, db: Session = Depends(get_db)):
    return crud.create_flight_data(db, data, flight_id)


@app.get("/api/flights")
def get_all_flights(db: Session = Depends(get_db)):
    return db.query(models.FlightSession).order_by(models.FlightSession.id.desc()).all()

@app.get("/api/data")
def get_data(flight_id: int, db: Session = Depends(get_db)):
    return db.query(models.FlightData).filter(models.FlightData.flight_id == flight_id).all()