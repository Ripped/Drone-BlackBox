from fastapi import FastAPI, Depends, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.orm import Session
from sqlalchemy import text
import datetime

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

hardware_state = {
    "delete_flight_id": -1,
    "clear_all": False
}

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


@app.post("/api/session/start")
def start_session(device_id: str, db: Session = Depends(get_db)):
    try:
        session = crud.start_new_session(db, device_id=device_id)
        current_date = datetime.datetime.utcnow().strftime("%Y-%m-%d")

        return {
            "flight_id": session.id,
            "date": current_date
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/api/data/{flight_id}")
def receive_data(
    flight_id: int,
    data: schemas.FlightDataCreate,
    db: Session = Depends(get_db)
):
    return crud.create_flight_data(db, data, flight_id)


@app.get("/api/flights")
def get_all_flights(db: Session = Depends(get_db)):
    return db.query(models.FlightSession).order_by(models.FlightSession.id.desc()).all()


@app.get("/api/data")
def get_data(flight_id: int = Query(...), db: Session = Depends(get_db)):
    return db.query(models.FlightData).filter(
        models.FlightData.flight_id == flight_id
    ).all()


@app.delete("/api/flights/{flight_id}")
def delete_single_flight(flight_id: int, db: Session = Depends(get_db)):
    active_session = (
        db.query(models.FlightSession)
        .order_by(models.FlightSession.id.desc())
        .first()
    )

    
    if active_session and flight_id == active_session.id:
        raise HTTPException(
            status_code=400,
            detail="Ne možete obrisati aktivni let."
        )

    hardware_state["delete_flight_id"] = flight_id

    db.query(models.FlightData).filter(
        models.FlightData.flight_id == flight_id
    ).delete()

    db.query(models.FlightSession).filter(
        models.FlightSession.id == flight_id
    ).delete()

    db.commit()

    return {
        "message": f"Flight {flight_id} deleted. Command sent to drone."
    }


@app.post("/api/hardware/clear-all")
def delete_all_flights(db: Session = Depends(get_db)):
    hardware_state["clear_all"] = True
    hardware_state["delete_flight_id"] = -1

    db.query(models.FlightData).delete()
    db.query(models.FlightSession).delete()

    
    try:
        db.execute(text("DELETE FROM sqlite_sequence WHERE name='flight_sessions'"))
        db.execute(text("DELETE FROM sqlite_sequence WHERE name='flight_data'"))
    except Exception as e:
        print("[SQLITE INFO]", e)

    db.commit()

    return {
        "message": "All flights cleared. SD wipe command sent."
    }


@app.get("/api/hardware/check-commands")
def check_hardware_commands():
    return hardware_state


@app.post("/api/hardware/confirm-delete")
def confirm_single_delete():
    hardware_state["delete_flight_id"] = -1
    return {"status": "delete reset"}


@app.post("/api/hardware/confirm-clear-all")
def confirm_clear_all():
    hardware_state["clear_all"] = False
    return {"status": "clear reset"}