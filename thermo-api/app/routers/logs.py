from fastapi import APIRouter, Depends, HTTPException, Header
from sqlalchemy.orm import Session
from datetime import datetime, timezone
from ..database import get_db
from ..models import Unit, Log
from ..schemas import LogBatch
from ..config import settings

router = APIRouter()

def verify_token(x_api_token: str = Header(...)):
    if x_api_token != settings.api_token:
        raise HTTPException(status_code=401, detail="Invalid token")

@router.post("/logs")
def post_logs(
    batch: LogBatch,
    db: Session = Depends(get_db),
    _: None = Depends(verify_token),
):
    ts_now = datetime.now(timezone.utc)

    unit = db.query(Unit).filter_by(mac=batch.unit_mac).first()
    unit_id = unit.id if unit else None

    for entry in batch.entries:
        log = Log(
            unit_id = unit_id,
            level   = entry.level.upper(),
            message = entry.message,
            ts      = entry.ts or ts_now,
        )
        db.add(log)

    db.commit()
    return {"accepted": len(batch.entries)}