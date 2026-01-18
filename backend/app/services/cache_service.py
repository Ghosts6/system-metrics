import json
import redis
from typing import Optional, Dict, Any
from app.config import settings


class CacheService:
    """Service for managing Redis cache operations."""
    
    def __init__(self):
        """Initialize Redis connection."""
        self.redis_client = redis.Redis(
            host=settings.redis_host,
            port=settings.redis_port,
            db=settings.redis_db,
            decode_responses=True,
            socket_connect_timeout=5,
            socket_timeout=5
        )
    
    def is_connected(self) -> bool:
        """Check if Redis is connected."""
        try:
            return self.redis_client.ping()
        except Exception:
            return False
    
    def set_latest_metrics(self, metrics: Dict[str, Any], ttl: Optional[int] = None) -> bool:
        """Store latest metrics snapshot in cache."""
        try:
            key = "metrics:latest"
            ttl = ttl or settings.cache_ttl
            self.redis_client.setex(
                key,
                ttl,
                json.dumps(metrics, default=str)
            )
            return True
        except Exception:
            return False
    
    def get_latest_metrics(self) -> Optional[Dict[str, Any]]:
        """Retrieve latest metrics from cache."""
        try:
            key = "metrics:latest"
            data = self.redis_client.get(key)
            if data:
                return json.loads(data)
            return None
        except Exception:
            return None
    
    def get_cache_stats(self) -> Dict[str, Any]:
        """Get cache statistics."""
        try:
            info = self.redis_client.info()
            return {
                "connected": True,
                "used_memory": info.get("used_memory_human", "N/A"),
                "connected_clients": info.get("connected_clients", 0),
            }
        except Exception:
            return {
                "connected": False,
                "error": "Redis not available"
            }


# Singleton instance
cache_service = CacheService()
