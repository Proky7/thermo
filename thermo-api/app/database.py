from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from influxdb_client import InfluxDBClient
from influxdb_client.client.write_api import SYNCHRONOUS
from .config import settings

# MariaDB
engine = create_engine(
    f"mysql+pymysql://{settings.db_user}:{settings.db_password}"
    f"@{settings.db_host}:{settings.db_port}/{settings.db_name}",
    pool_pre_ping=True,      # detekuje ztracená spojení
    pool_recycle=3600,
)
SessionLocal = sessionmaker(bind=engine)

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

# InfluxDB
influx_client = InfluxDBClient(
    url=settings.influx_url,
    token=settings.influx_token,
    org=settings.influx_org,
)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)