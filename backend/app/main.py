from contextlib import asynccontextmanager
from fastapi import FastAPI, Request, status
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware
from loguru import logger
from sqlalchemy.orm import Session
from app.config import settings
from app.database import engine, Base, SessionLocal
from app.routers import metrics, logs
from app.services.cache_service import cache_service
from app.logging_config import setup_logging
from app.exceptions import APIError
from app.models import SystemMetrics

# Setup logging
setup_logging(settings)

# Create database tables
Base.metadata.create_all(bind=engine)


@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info("Application startup")
    if cache_service.is_connected():
        logger.info("✓ Redis connection established")
        # Cache warming
        db: Session = SessionLocal()
        try:
            latest_metrics = db.query(SystemMetrics).order_by(SystemMetrics.timestamp.desc()).first()
            if latest_metrics:
                cache_service.set_latest_metrics(latest_metrics.to_dict())
                logger.info("Cache warmed with latest metrics.")
            else:
                logger.info("No metrics found to warm the cache.")
        except Exception as e:
            logger.error(f"Failed to warm cache: {e}", exc_info=True)
        finally:
            db.close()
    else:
        logger.warning("⚠ Redis connection unavailable (continuING without cache)")
    yield
    logger.info("Application shutdown")


# Initialize FastAPI app
app = FastAPI(
    title="System Metrics & Log Analytics Platform",
    description="A production-grade service for system monitoring.",
    version="0.1.0",
    docs_url="/docs",
    redoc_url="/redoc",
    lifespan=lifespan,
)

# CORS middleware (configure as needed for your Qt client)
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In production, specify actual origins
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Exception handler for custom APIError
@app.exception_handler(APIError)
async def api_error_handler(request: Request, exc: APIError):
    logger.error(f"API Error caught: {exc.detail}", exc_info=True)
    return JSONResponse(
        status_code=exc.status_code,
        content={"detail": exc.detail}
    )

# Include routers
app.include_router(metrics.router, prefix=settings.api_v1_prefix)
app.include_router(logs.router, prefix=settings.api_v1_prefix)


@app.get("/")
def read_root():
    """Root endpoint to confirm the service is running."""
    return {
        "status": "ok",
        "message": "System Metrics & Log Analytics Platform",
        "version": "0.1.0",
        "docs": "/docs"
    }


@app.get("/api/v1/health")
def read_health():
    redis_status = "ok" if cache_service.is_connected() else "unavailable"
    
    return {
        "status": "ok",
        "database": "ok",
        "redis": redis_status
    }
