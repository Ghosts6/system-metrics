import json
import redis
from typing import Optional, Dict, Any
from loguru import logger
from app.config import settings


class CacheService:
    """Service for managing Redis cache operations."""
    
    def __init__(self):
        """Initialize Redis connection."""
        try:
            self.redis_client = redis.Redis(
                host=settings.redis_host,
                port=settings.redis_port,
                db=settings.redis_db,
                decode_responses=True,
                socket_connect_timeout=5,
                socket_timeout=5
            )
            self.redis_client.ping()
            logger.info("Redis connection established.")
        except redis.exceptions.ConnectionError as e:
            logger.error(f"Redis connection failed: {e}", exc_info=True)
            self.redis_client = None
    
    def is_connected(self) -> bool:
        """Check if Redis is connected."""
        if not self.redis_client:
            return False
        try:
            return self.redis_client.ping()
        except redis.exceptions.ConnectionError as e:
            logger.error(f"Redis connection error: {e}", exc_info=True)
            return False
    
    def set_latest_metrics(self, metrics: Dict[str, Any], ttl: Optional[int] = None) -> bool:
        """Store latest metrics snapshot in cache."""
        if not self.is_connected():
            return False
        try:
            key = "metrics:latest"
            ttl = ttl or settings.cache_ttl
            self.redis_client.setex(
                key,
                ttl,
                json.dumps(metrics, default=str)
            )
            logger.debug(f"Cached latest metrics with key: {key}")
            return True
        except Exception as e:
            logger.error(f"Failed to set latest metrics in cache: {e}", exc_info=True)
            raise CacheError(detail=f"Failed to set latest metrics in cache: {str(e)}")
    
    def get_latest_metrics(self) -> Optional[Dict[str, Any]]:
        """Retrieve latest metrics from cache."""
        if not self.is_connected():
            return None
        try:
            key = "metrics:latest"
            data = self.redis_client.get(key)
            if data:
                logger.debug(f"Cache hit for key: {key}")
                return json.loads(data)
            logger.debug(f"Cache miss for key: {key}")
            return None
        except Exception as e:
            logger.error(f"Failed to get latest metrics from cache: {e}", exc_info=True)
            raise CacheError(detail=f"Failed to get latest metrics from cache: {str(e)}")
    
    def get_cache_stats(self) -> Dict[str, Any]:
        """Get cache statistics."""
        if not self.is_connected():
            return {"connected": False, "error": "Redis not available"}
        try:
            info = self.redis_client.info()
            return {
                "connected": True,
                "used_memory": info.get("used_memory_human", "N/A"),
                "connected_clients": info.get("connected_clients", 0),
            }
        except Exception as e:
            logger.error(f"Failed to get cache stats: {e}", exc_info=True)
            return {
                "connected": False,
                "error": "Redis not available"
            }


# Singleton instance
cache_service = CacheService()
