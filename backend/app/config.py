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
    
    # Application settings
    secret_key: str = os.getenv("SECRET_KEY", "dev-secret-key-change-in-production")
    api_v1_prefix: str = "/api/v1"
    
    # Metrics collection settings
    metrics_collection_interval: int = int(os.getenv("METRICS_COLLECTION_INTERVAL", "5"))  # seconds
    metrics_retention_days: int = int(os.getenv("METRICS_RETENTION_DAYS", "30"))
    metrics_collector: str = os.getenv("METRICS_COLLECTOR", "psutil")  # psutil or c-collector
    c_collector_path: Optional[str] = os.getenv("C_COLLECTOR_PATH", None)  # Path to C collector binary
    
    model_config = ConfigDict(
        env_file=".env",
        case_sensitive=False
    )


settings = Settings()
