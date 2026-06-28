from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
from ..config import settings
import logging

logger = logging.getLogger(__name__)

client = InfluxDBClient(
    url=settings.influx_url,
    token=settings.influx_token,
    org=settings.influx_org,
)
write_api = client.write_api(write_options=SYNCHRONOUS)

def write_measurements(points: list[Point]):
    try:
        write_api.write(bucket=settings.influx_bucket, record=points)
    except Exception as e:
        logger.error(f"InfluxDB write failed: {e}")
        raise