from pydantic import BaseModel
from typing import Optional
from datetime import datetime

# --- Measurements ---
class SensorReading(BaseModel):
    rom_id: str        # "28FF1234ABCD0000"
    value: float       # °C

class MeasurementBatch(BaseModel):
    unit_mac:  str
    unit_name: str
    fw_version: Optional[str] = None
    readings:  list[SensorReading]
    ts:        Optional[datetime] = None   # pokud None → server čas

# --- Sensors ---
class SensorAnnounce(BaseModel):
    unit_mac:  str
    unit_name: str
    rom_id:    str
    name:      Optional[str] = None        # pokud ESP posílá název

class SensorNameUpdate(BaseModel):
    name: str

class SensorOut(BaseModel):
    rom_id: str
    name:   Optional[str]

class SensorListOut(BaseModel):
    sensors: list[SensorOut]

# --- Logs ---
class LogEntry(BaseModel):
    level:   str
    message: str
    ts:      Optional[datetime] = None

class LogBatch(BaseModel):
    unit_mac:  str
    unit_name: str
    entries:   list[LogEntry]