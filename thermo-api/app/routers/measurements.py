from fastapi import APIRouter, Depends, HTTPException, Header
from sqlalchemy.orm import Session
from datetime import datetime, timezone
from ..database import get_db, write_api
from ..models import Unit, Sensor
from ..schemas import MeasurementBatch
from ..config import settings
from influxdb_client import Point
import logging

router = APIRouter()
logger = logging.getLogger(__name__)

def verify_token(x_api_token: str = Header(...)):
    if x_api_token != settings.api_token:
        raise HTTPException(status_code=401, detail="Invalid token")

@router.post("/measurements")
def post_measurements(
    batch: MeasurementBatch,
    db: Session = Depends(get_db),
    _: None = Depends(verify_token),
):
    ts = batch.ts or datetime.now(timezone.utc)

    # Upsert unit
    unit = db.query(Unit).filter_by(mac=batch.unit_mac).first()
    if not unit:
        unit = Unit(mac=batch.unit_mac, name=batch.unit_name)
        db.add(unit)
        db.flush()
    unit.name      = batch.unit_name
    unit.fw_version = batch.fw_version
    unit.last_seen = ts

    points = []
    skipped = 0

    for r in batch.readings:
        # Zkontroluj jestli senzor má jméno
        sensor = db.query(Sensor).filter_by(
            unit_id=unit.id, rom_id=r.rom_id
        ).first()

        if not sensor or not sensor.name:
            skipped += 1
            logger.debug(f"Skipping unnamed sensor {r.rom_id}")
            continue

        sensor.last_seen = ts

        points.append(
            Point("temperature")
            .tag("unit_id",     batch.unit_mac[-6:])   # suffix MAC
            .tag("unit_name",   batch.unit_name)
            .tag("sensor_id",   r.rom_id)
            .tag("sensor_name", sensor.name)
            .field("value",     r.value)
            .time(ts)
        )

    if points:
        write_api.write(bucket=settings.influx_bucket, record=points)

    db.commit()

    return {
        "accepted": len(points),
        "skipped":  skipped,
        "ts":       ts.isoformat(),
    }