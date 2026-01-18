from datetime import datetime
from typing import Optional
from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.orm import Session
from sqlalchemy import desc
from app.database import get_db
from app.schemas import SystemLogCreate, SystemLogResponse, SystemLogListResponse
from app.models import SystemLog

router = APIRouter(prefix="/logs", tags=["logs"])


@router.post("/", response_model=SystemLogResponse, status_code=201)
def create_log(log: SystemLogCreate, db: Session = Depends(get_db)):
    """
    Create a new system log entry.
    """
    try:
        db_log = SystemLog(**log.model_dump())
        db.add(db_log)
        db.commit()
        db.refresh(db_log)
        return db_log
    except Exception as e:
        db.rollback()
        raise HTTPException(status_code=500, detail=f"Failed to create log: {str(e)}")


@router.get("/", response_model=SystemLogListResponse)
def get_logs(
    level: Optional[str] = Query(None, description="Filter by log level"),
    start_time: Optional[datetime] = Query(None, description="Start time for filtering"),
    end_time: Optional[datetime] = Query(None, description="End time for filtering"),
    page: int = Query(1, ge=1, description="Page number"),
    page_size: int = Query(100, ge=1, le=1000, description="Items per page"),
    db: Session = Depends(get_db)
):
    """
    Get system logs with filtering and pagination.
    """
    try:
        query = db.query(SystemLog)
        
        if level:
            query = query.filter(SystemLog.level == level.upper())
        if start_time:
            query = query.filter(SystemLog.timestamp >= start_time)
        if end_time:
            query = query.filter(SystemLog.timestamp <= end_time)
        
        # Get total count
        total = query.count()
        
        # Apply pagination
        offset = (page - 1) * page_size
        logs = query.order_by(desc(SystemLog.timestamp)).offset(offset).limit(page_size).all()
        
        pages = (total + page_size - 1) // page_size if total > 0 else 0
        
        return SystemLogListResponse(
            items=logs,
            total=total,
            page=page,
            page_size=page_size,
            pages=pages
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to retrieve logs: {str(e)}")


@router.get("/{log_id}", response_model=SystemLogResponse)
def get_log(log_id: int, db: Session = Depends(get_db)):
    """
    Get a specific log entry by ID.
    """
    log = db.query(SystemLog).filter(SystemLog.id == log_id).first()
    if not log:
        raise HTTPException(status_code=404, detail="Log not found")
    return log
