from sqlalchemy import Column, Integer, String, DateTime, Text, Enum, ForeignKey
from sqlalchemy.orm import declarative_base, relationship
from datetime import datetime

Base = declarative_base()

class Unit(Base):
    __tablename__ = "thermo_units"
    id         = Column(Integer, primary_key=True)
    mac        = Column(String(17), unique=True, nullable=False)
    name       = Column(String(64))
    fw_version = Column(String(32))
    last_seen  = Column(DateTime)
    created_at = Column(DateTime, default=datetime.utcnow)
    sensors    = relationship("Sensor", back_populates="unit")

class Sensor(Base):
    __tablename__ = "thermo_sensors"
    id         = Column(Integer, primary_key=True)
    unit_id    = Column(Integer, ForeignKey("thermo_units.id"), nullable=False)
    rom_id     = Column(String(32), nullable=False)
    name       = Column(String(64))
    last_seen  = Column(DateTime)
    created_at = Column(DateTime, default=datetime.utcnow)
    unit       = relationship("Unit", back_populates="sensors")

class Log(Base):
    __tablename__ = "thermo_logs"
    id         = Column(Integer, primary_key=True)
    unit_id    = Column(Integer, ForeignKey("thermo_units.id"), nullable=True)
    level      = Column(Enum("DEBUG","INFO","WARN","ERROR","CRITICAL"))
    message    = Column(Text)
    ts         = Column(DateTime, nullable=False)
    created_at = Column(DateTime, default=datetime.utcnow)