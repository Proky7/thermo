from sqlalchemy.orm import Session
from ..models import Unit, Sensor
from datetime import datetime, timezone
import logging

logger = logging.getLogger(__name__)

def upsert_unit(db: Session, mac: str, name: str, fw_version: str = None) -> Unit:
    unit = db.query(Unit).filter_by(mac=mac).first()
    if not unit:
        logger.info(f"New unit registered: {name} ({mac})")
        unit = Unit(mac=mac, name=name)
        db.add(unit)
        db.flush()
    unit.name = name
    unit.fw_version = fw_version
    unit.last_seen = datetime.now(timezone.utc)
    return unit

def upsert_sensor(db: Session, unit: Unit, rom_id: str) -> Sensor:
    sensor = db.query(Sensor).filter_by(
        unit_id=unit.id, rom_id=rom_id
    ).first()
    if not sensor:
        logger.info(f"New sensor discovered: {rom_id} on unit {unit.name}")
        sensor = Sensor(unit_id=unit.id, rom_id=rom_id)
        db.add(sensor)
    sensor.last_seen = datetime.now(timezone.utc)
    return sensor

def get_named_sensors(db: Session, unit: Unit) -> dict[str, str]:
    """Vrátí dict {rom_id: name} pouze pro pojmenované senzory."""
    sensors = db.query(Sensor).filter(
        Sensor.unit_id == unit.id,
        Sensor.name.isnot(None)
    ).all()
    return {s.rom_id: s.name for s in sensors}