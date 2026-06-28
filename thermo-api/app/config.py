from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    # MariaDB
    db_host: str = "192.168.5.x"   # Synology IP
    db_port: int = 3306
    db_name: str = "HomeSys"
    db_user: str = "thermo"
    db_password: str

    # InfluxDB
    influx_url: str = "http://influxdb-svc.default.svc.cluster.local:8086"
    influx_org: str = "HomeSys"
    influx_bucket: str = "teploty"
    influx_token: str

    # API
    api_token: str              # jednoduchý shared secret pro ESP
    log_level: str = "INFO"

    class Config:
        env_file = ".env"
        case_sensitive = False

settings = Settings()