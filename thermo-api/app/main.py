from fastapi import FastAPI
from .database import engine
from .models import Base
from .routers import measurements, sensors, logs
import logging

logging.basicConfig(level=logging.INFO)

app = FastAPI(title="Thermo API", version="0.1.0")

@app.on_event("startup")
def startup():
    Base.metadata.create_all(bind=engine)  # vytvoří tabulky pokud neexistují

app.include_router(measurements.router, prefix="/api/v1")
app.include_router(sensors.router,      prefix="/api/v1")
app.include_router(logs.router,         prefix="/api/v1")

@app.get("/health")
def health():
    return {"status": "ok"}