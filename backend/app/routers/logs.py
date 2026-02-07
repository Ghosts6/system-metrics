from datetime import datetime
from typing import Optional
from fastapi import APIRouter, Depends, Query, status
from sqlalchemy.orm import Session
from sqlalchemy import desc, or_
from loguru import logger
from app.database import get_db
from app.schemas import SystemLogCreate, SystemLogResponse, SystemLogListResponse
from app.models import SystemLog
from app.exceptions import DatabaseError, LogNotFoundError

router = APIRouter(prefix="/logs", tags=["logs"])


@router.post("/", response_model=SystemLogResponse, status_code=status.HTTP_201_CREATED)
def create_log(log: SystemLogCreate, db: Session = Depends(get_db)):
    """
    Create a new system log entry.
    """
    try:
        db_log = SystemLog(**log.model_dump())
        db.add(db_log)
        db.commit()
        db.refresh(db_log)
        logger.info(f"Created new log with ID: {db_log.id}")
        return db_log
    except Exception as e:
        db.rollback()
        logger.error(f"Failed to create log: {e}", exc_info=True)
        raise DatabaseError(detail=f"Failed to create log: {str(e)}")


@router.get("/", response_model=SystemLogListResponse)
def get_logs(
    level: Optional[str] = Query(None, description="Filter by log level"),
    start_time: Optional[datetime] = Query(None, description="Start time for filtering"),
    end_time: Optional[datetime] = Query(None, description="End time for filtering"),
    search: Optional[str] = Query(None, description="Search in message and source fields"),
    page: int = Query(1, ge=1, description="Page number"),
    page_size: int = Query(100, ge=1, le=1000, description="Items per page"),
    db: Session = Depends(get_db)
):
    """
    Get system logs with filtering and pagination.
    Supports searching in message and source fields.
    """
    try:
        query = db.query(SystemLog)
        
        if level:
            query = query.filter(SystemLog.level == level.upper())
        if start_time:
            query = query.filter(SystemLog.timestamp >= start_time)
        if end_time:
            query = query.filter(SystemLog.timestamp <= end_time)
        if search:
            # Search in both message and source fields (case-insensitive)
            search_pattern = f"%{search}%"
            query = query.filter(
                or_(
                    SystemLog.message.ilike(search_pattern),
                    SystemLog.source.ilike(search_pattern)
                )
            )
        
        # Get total count
        total = query.count()
        
        # Apply pagination
        offset = (page - 1) * page_size
        logs = query.order_by(desc(SystemLog.timestamp)).offset(offset).limit(page_size).all()
        
        pages = (total + page_size - 1) // page_size if total > 0 else 0
        
        logger.debug(f"Retrieved {len(logs)} logs for page {page} of {pages} total pages.")
        
        return SystemLogListResponse(
            items=logs,
            total=total,
            page=page,
            page_size=page_size,
            pages=pages
        )
    except Exception as e:
        logger.error(f"Failed to retrieve logs: {e}", exc_info=True)
        raise DatabaseError(detail=f"Failed to retrieve logs: {str(e)}")


@router.get("/{log_id}", response_model=SystemLogResponse)
def get_log(log_id: int, db: Session = Depends(get_db)):
    """
    Get a specific log entry by ID.
    """
    log = db.query(SystemLog).filter(SystemLog.id == log_id).first()
    if not log:
        logger.warning(f"Log with ID {log_id} not found.")
        raise LogNotFoundError(detail=f"Log entry with ID {log_id} not found")
    logger.debug(f"Retrieved log with ID: {log_id}")
    return log
