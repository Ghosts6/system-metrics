import os
from typing import Optional
from pydantic_settings import BaseSettings
from pydantic import ConfigDict


class Settings(BaseSettings):
    """Application settings loaded from environment variables."""
    
    # Database settings
    database_url: str = os.getenv(
        "DATABASE_URL",
        "sqlite:///./system_metrics.db"
    )
    
    # Redis settings
    redis_host: str = os.getenv("REDIS_HOST", "localhost")
    redis_port: int = int(os.getenv("REDIS_PORT", "6379"))
    redis_db: int = int(os.getenv("REDIS_DB", "0"))
    cache_ttl: int = int(os.getenv("CACHE_TTL", "60")) # seconds
    
    # Application settings
    secret_key: str = os.getenv("SECRET_KEY", "dev-secret-key-change-in-production")
    api_url: str = os.getenv("API_URL", "http://localhost:8000") # Base URL for the API
    api_v1_prefix: str = "/api/v1"
    
    # Metrics collection settings
    metrics_collection_interval: int = int(os.getenv("METRICS_COLLECTION_INTERVAL", "5"))  # seconds
    metrics_retention_days: int = int(os.getenv("METRICS_RETENTION_DAYS", "30"))
    metrics_collector: str = os.getenv("METRICS_COLLECTOR", "psutil")  # psutil or c-collector
    c_collector_path: Optional[str] = os.getenv("C_COLLECTOR_PATH", None)  # Path to C collector binary
    log_to_api_min_level: Optional[str] = os.getenv("LOG_TO_API_MIN_LEVEL", None) # Minimum level for sending logs to API (e.g., "ERROR", "WARNING")
    
    model_config = ConfigDict(
        env_file=".env",
        case_sensitive=False
    )


settings = Settings()
