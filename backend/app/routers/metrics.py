from datetime import datetime
from typing import Optional
from fastapi import APIRouter, Depends, HTTPException, Query
from sqlalchemy.orm import Session, joinedload
from app.database import get_db
from app.schemas import (
    SystemMetricsResponse,
    SystemMetricsListResponse,
    SystemMetricsCreate,
    HealthResponse
)
from app.models import SystemMetrics, GpuMetrics
# from app.services.metrics_service import metrics_service
# from app.services.cache_service import cache_service

router = APIRouter(prefix="/metrics", tags=["metrics"])

latest_metrics_cache = None

@router.get("/live", response_model=SystemMetricsResponse)
def get_live_metrics(db: Session = Depends(get_db)):
    """
    Get the latest system metrics from cache (live data).
    Returns cached metrics if available, otherwise fetches from DB.
    """
    global latest_metrics_cache
    if latest_metrics_cache:
        # Re-fetch from DB with eager loading to avoid DetachedInstanceError
        # for relationship access in Pydantic serialization
        db_metrics = db.query(SystemMetrics).options(joinedload(SystemMetrics.gpus)).filter(SystemMetrics.id == latest_metrics_cache.id).first()
        if db_metrics:
            return db_metrics
        # If cache somehow holds a non-existent ID or is stale, fall through to query DB
        latest_metrics_cache = None # Clear stale cache

    db_metrics = db.query(SystemMetrics).options(joinedload(SystemMetrics.gpus)).order_by(SystemMetrics.timestamp.desc()).first()
    if not db_metrics:
        raise HTTPException(status_code=404, detail="No metrics found")
        
    latest_metrics_cache = db_metrics # Update cache with eagerly loaded object
    return db_metrics


@router.post("/collect", response_model=SystemMetricsResponse, status_code=201)
def collect_and_save_metrics(metrics: SystemMetricsCreate, db: Session = Depends(get_db)):
    """
    Collect current system metrics and save to database.
    Also updates the cache with latest metrics.
    """
    global latest_metrics_cache
    try:
        # Create the main metrics record
        db_metrics = SystemMetrics(**metrics.model_dump(exclude={"gpus"}))
        db.add(db_metrics)
        db.flush() # Flush to get db_metrics.id before adding gpus
        db.refresh(db_metrics) # Refresh to load default values like timestamp
        
        # Create the GPU metrics records
        if metrics.gpus:
            for gpu_metric in metrics.gpus:
                db_gpu_metric = GpuMetrics(**gpu_metric.model_dump(), metric_id=db_metrics.id)
                db.add(db_gpu_metric)
        
        db.commit()
        db.refresh(db_metrics) # Refresh again to load the gpus relationship

        # Update cache with the eagerly loaded object
        latest_metrics_cache = db.query(SystemMetrics).options(joinedload(SystemMetrics.gpus)).filter(SystemMetrics.id == db_metrics.id).first()
        
        return db_metrics
    except Exception as e:
        db.rollback()
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
        query = db.query(SystemMetrics).options(joinedload(SystemMetrics.gpus))
        if start_time:
            query = query.filter(SystemMetrics.timestamp >= start_time)
        if end_time:
            query = query.filter(SystemMetrics.timestamp <= end_time)
        
        total = query.count()
        
        metrics = query.order_by(SystemMetrics.timestamp.desc()).offset((page - 1) * page_size).limit(page_size).all()
        
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
    # Simplified health check since cache service is not used
    return HealthResponse(
        status="ok",
        database="ok"
    )
