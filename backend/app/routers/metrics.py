from datetime import datetime
from typing import Optional
from fastapi import APIRouter, Depends, Query, status
from sqlalchemy.orm import Session, joinedload
from loguru import logger
from app.database import get_db
from app.schemas import (
    SystemMetricsResponse,
    SystemMetricsListResponse,
    SystemMetricsCreate,
    HealthResponse
)
from app.models import SystemMetrics, GpuMetrics
from app.services.cache_service import cache_service
from app.exceptions import DatabaseError, CacheError, MetricsNotFoundError, APIError

router = APIRouter(prefix="/metrics", tags=["metrics"])

@router.get("/live", response_model=SystemMetricsResponse)
def get_live_metrics(db: Session = Depends(get_db)):
    """
    Get the latest system metrics from cache (live data).
    Returns cached metrics if available, otherwise fetches from DB.
    """
    try:
        cached_metrics = cache_service.get_latest_metrics()
        if cached_metrics:
            logger.info("Cache hit for live metrics.")
            # Re-fetch from DB with eager loading to avoid DetachedInstanceError
            db_metrics = db.query(SystemMetrics).options(joinedload(SystemMetrics.gpus)).filter(SystemMetrics.id == cached_metrics['id']).first()
            if db_metrics:
                return db_metrics
            logger.warning("Cached metrics ID not found in DB, fetching fresh data.")
        
        logger.info("Cache miss for live metrics, fetching from DB.")
        db_metrics = db.query(SystemMetrics).options(joinedload(SystemMetrics.gpus)).order_by(SystemMetrics.timestamp.desc()).first()
        if not db_metrics:
            logger.warning("No metrics found in the database.")
            raise MetricsNotFoundError(detail="No metrics found")
            
        cache_service.set_latest_metrics(db_metrics.to_dict())
        return db_metrics
    except MetricsNotFoundError:
        raise
    except CacheError:
        raise # Re-raise CacheError directly
    except Exception as e:
        logger.error(f"Failed to retrieve live metrics: {e}", exc_info=True)
        raise DatabaseError(detail=f"Failed to retrieve live metrics: {str(e)}")


@router.post("/collect", response_model=SystemMetricsResponse, status_code=status.HTTP_201_CREATED)
def collect_and_save_metrics(metrics: SystemMetricsCreate, db: Session = Depends(get_db)):
    """
    Collect current system metrics and save to database.
    Also updates the cache with latest metrics.
    """
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

        # Update cache
        try:
            cache_service.set_latest_metrics(db_metrics.to_dict())
        except CacheError as e:
            logger.warning(f"Failed to update cache with latest metrics: {e.detail}")
        
        logger.info(f"Collected and saved new metrics with ID: {db_metrics.id}")
        
        return db_metrics
    except CacheError:
        raise # Re-raise CacheError directly
    except Exception as e:
        db.rollback()
        logger.error(f"Failed to collect and save metrics: {e}", exc_info=True)
        raise DatabaseError(detail=f"Failed to collect and save metrics: {str(e)}")


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
        
        logger.debug(f"Retrieved {len(metrics)} metrics for page {page} of {pages} total pages.")
        
        return SystemMetricsListResponse(
            items=metrics,
            total=total,
            page=page,
            page_size=page_size,
            pages=pages
        )
    except Exception as e:
        logger.error(f"Failed to retrieve metrics history: {e}", exc_info=True)
        raise DatabaseError(detail=f"Failed to retrieve metrics history: {str(e)}")


@router.get("/health", response_model=HealthResponse)
def metrics_health():
    """
    Health check for metrics service.
    Checks database and Redis connectivity.
    """
    redis_status = "ok"
    if not cache_service.is_connected():
        redis_status = "unavailable"
        logger.warning("Redis is unavailable during health check.")

    logger.info(f"Health check: DB is ok, Redis is {redis_status}")
    return HealthResponse(
        status="ok",
        database="ok",
        redis=redis_status
    )
