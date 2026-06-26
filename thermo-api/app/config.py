from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    # MariaDB
    db_host: str = "192.168.5.x"   # Synology IP
    db_port: int = 3306
    db_name: str = "thermo"
    db_user: str = "thermo"
    db_password: str

    # InfluxDB
    influx_url: str = "http://influxdb.monitoring.svc.cluster.local:8086"
    influx_token: str
    influx_org: str = "homelab"
    influx_bucket: str = "temperatures"

    # API
    api_token: str              # jednoduchý shared secret pro ESP
    log_level: str = "INFO"

    class Config:
        env_file = ".env"

settings = Settings()