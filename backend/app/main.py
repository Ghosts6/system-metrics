from contextlib import asynccontextmanager
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.config import settings
from app.database import engine, Base
from app.routers import metrics, logs
from app.services.cache_service import cache_service

# Create database tables
Base.metadata.create_all(bind=engine)


@asynccontextmanager
async def lifespan(app: FastAPI):
    if cache_service.is_connected():
        print("✓ Redis connection established")
    else:
        print("⚠ Redis connection unavailable (continuing without cache)")
    yield
    pass


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
