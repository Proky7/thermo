from fastapi import APIRouter, Depends, HTTPException, Header
from sqlalchemy.orm import Session
from datetime import datetime, timezone
from ..database import get_db
from ..models import Unit, Sensor
from ..schemas import SensorAnnounce, SensorNameUpdate, SensorListOut, SensorOut
from ..config import settings

router = APIRouter()

def verify_token(x_api_token: str = Header(...)):
    if x_api_token != settings.api_token:
        raise HTTPException(status_code=401, detail="Invalid token")

@router.get("/sensors/{unit_mac}", response_model=SensorListOut)
def get_sensors(
    unit_mac: str,
    db: Session = Depends(get_db),
    _: None = Depends(verify_token),
):
    unit = db.query(Unit).filter_by(mac=unit_mac).first()
    if not unit:
        return SensorListOut(sensors=[])

    sensors = db.query(Sensor).filter_by(unit_id=unit.id).all()
    return SensorListOut(sensors=[
        SensorOut(rom_id=s.rom_id, name=s.name) for s in sensors
    ])

@router.post("/sensors")
def announce_sensor(
    body: SensorAnnounce,
    db: Session = Depends(get_db),
    _: None = Depends(verify_token),
):
    # Upsert unit
    unit = db.query(Unit).filter_by(mac=body.unit_mac).first()
    if not unit:
        unit = Unit(mac=body.unit_mac, name=body.unit_name)
        db.add(unit)
        db.flush()

    # Upsert sensor
    sensor = db.query(Sensor).filter_by(
        unit_id=unit.id, rom_id=body.rom_id
    ).first()

    if not sensor:
        sensor = Sensor(unit_id=unit.id, rom_id=body.rom_id)
        db.add(sensor)

    # Jméno nastavíme jen pokud přichází a sensor ještě jméno nemá
    # (MariaDB je master — nepřepisujeme existující jméno z ESP)
    if body.name and not sensor.name:
        sensor.name = body.name

    sensor.last_seen = datetime.now(timezone.utc)
    db.commit()

    return {"rom_id": sensor.rom_id, "name": sensor.name}

@router.patch("/sensors/{unit_mac}/{rom_id}")
def update_sensor_name(
    unit_mac: str,
    rom_id: str,
    body: SensorNameUpdate,
    db: Session = Depends(get_db),
    _: None = Depends(verify_token),
):
    unit = db.query(Unit).filter_by(mac=unit_mac).first()
    if not unit:
        raise HTTPException(status_code=404, detail="Unit not found")

    sensor = db.query(Sensor).filter_by(
        unit_id=unit.id, rom_id=rom_id
    ).first()
    if not sensor:
        raise HTTPException(status_code=404, detail="Sensor not found")

    sensor.name = body.name
    db.commit()
    return {"rom_id": sensor.rom_id, "name": sensor.name}