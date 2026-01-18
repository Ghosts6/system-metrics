from datetime import datetime
from typing import Optional
from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.orm import Session
from app.database import get_db
from app.schemas import (
    SystemMetricsResponse,
    SystemMetricsListResponse,
    SystemMetricsCreate,
    HealthResponse
)
from app.services.metrics_service import metrics_service
from app.services.cache_service import cache_service

router = APIRouter(prefix="/metrics", tags=["metrics"])


@router.get("/live", response_model=dict)
def get_live_metrics():
    """
    Get the latest system metrics from cache (live data).
    Returns cached metrics if available, otherwise collects fresh metrics.
    """
    # Try to get from cache first
    cached_metrics = metrics_service.get_latest_metrics()
    if cached_metrics:
        return cached_metrics
    
    # If not in cache, collect fresh metrics
    metrics = metrics_service.collect_metrics()
    metrics_dict = metrics.model_dump()
    metrics_dict['timestamp'] = datetime.utcnow().isoformat()
    
    # Cache it for future requests
    metrics_service.cache_latest_metrics(metrics)
    
    return metrics_dict


@router.post("/collect", response_model=SystemMetricsResponse, status_code=201)
def collect_and_save_metrics(db: Session = Depends(get_db)):
    """
    Collect current system metrics and save to database.
    Also updates the cache with latest metrics.
    """
    try:
        # Collect metrics
        metrics = metrics_service.collect_metrics()
        
        # Save to database
        db_metrics = metrics_service.save_metrics(db, metrics)
        
        # Update cache
        metrics_service.cache_latest_metrics(metrics)
        
        return db_metrics
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to collect metrics: {str(e)}")


@router.get("/history", response_model=SystemMetricsListResponse)
def get_metrics_history(
    start_time: Optional[datetime] = Query(None, description="Start time for filtering"),
    end_time: Optional[datetime] = Query(None, description="End time for filtering"),
    page: int = Query(1, ge=1, description="Page number"),
    page_size: int = Query(100, ge=1, le=1000, description="Items per page"),
    db: Session = Depends(get_db)
):
    """
    Get historical system metrics with pagination.
    Supports filtering by time range.
    """
    try:
        metrics, total = metrics_service.get_metrics_history(
            db=db,
            start_time=start_time,
            end_time=end_time,
            page=page,
            page_size=page_size
        )
        
        pages = (total + page_size - 1) // page_size if total > 0 else 0
        
        return SystemMetricsListResponse(
            items=metrics,
            total=total,
            page=page,
            page_size=page_size,
            pages=pages
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to retrieve metrics history: {str(e)}")


@router.get("/health", response_model=HealthResponse)
def metrics_health():
    """
    Health check for metrics service.
    Checks database and Redis connectivity.
    """
    redis_status = "ok" if cache_service.is_connected() else "unavailable"
    
    return HealthResponse(
        status="ok",
        redis=redis_status
    )
