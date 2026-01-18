import psutil
import socket
import platform
from datetime import datetime
from typing import Dict, Any, Optional
from app.schemas import SystemMetricsCreate
from app.models import SystemMetrics
from app.services.cache_service import cache_service
from sqlalchemy.orm import Session


class MetricsService:
    """Service for collecting and managing system metrics."""
    
    @staticmethod
    def collect_metrics() -> SystemMetricsCreate:
        """Collect current system metrics using psutil."""
        # CPU metrics
        cpu_percent = psutil.cpu_percent(interval=0.1)
        cpu_count = psutil.cpu_count()
        cpu_freq = psutil.cpu_freq()
        cpu_freq_current = cpu_freq.current if cpu_freq else None
        cpu_freq_min = cpu_freq.min if cpu_freq else None
        cpu_freq_max = cpu_freq.max if cpu_freq else None
        
        # Memory metrics
        memory = psutil.virtual_memory()
        memory_total = memory.total
        memory_available = memory.available
        memory_used = memory.used
        memory_percent = memory.percent
        
        # Disk metrics (using root partition)
        disk = psutil.disk_usage('/')
        disk_total = disk.total
        disk_used = disk.used
        disk_free = disk.free
        disk_percent = (disk.used / disk.total * 100) if disk.total > 0 else 0
        
        # Network metrics (aggregated)
        network_io = psutil.net_io_counters()
        network_bytes_sent = network_io.bytes_sent if network_io else None
        network_bytes_recv = network_io.bytes_recv if network_io else None
        
        # System info
        hostname = socket.gethostname()
        platform_name = platform.system()
        
        return SystemMetricsCreate(
            cpu_percent=cpu_percent,
            cpu_count=cpu_count,
            cpu_freq_current=cpu_freq_current,
            cpu_freq_min=cpu_freq_min,
            cpu_freq_max=cpu_freq_max,
            memory_total=memory_total,
            memory_available=memory_available,
            memory_used=memory_used,
            memory_percent=memory_percent,
            disk_total=disk_total,
            disk_used=disk_used,
            disk_free=disk_free,
            disk_percent=disk_percent,
            network_bytes_sent=network_bytes_sent,
            network_bytes_recv=network_bytes_recv,
            hostname=hostname,
            platform=platform_name
        )
    
    @staticmethod
    def save_metrics(db: Session, metrics: SystemMetricsCreate) -> SystemMetrics:
        """Save metrics to database."""
        db_metrics = SystemMetrics(**metrics.model_dump())
        db.add(db_metrics)
        db.commit()
        db.refresh(db_metrics)
        return db_metrics
    
    @staticmethod
    def cache_latest_metrics(metrics: SystemMetricsCreate) -> bool:
        """Cache latest metrics in Redis."""
        metrics_dict = metrics.model_dump()
        metrics_dict['timestamp'] = datetime.utcnow().isoformat()
        return cache_service.set_latest_metrics(metrics_dict)
    
    @staticmethod
    def get_latest_metrics() -> Optional[Dict[str, Any]]:
        """Get latest metrics from cache."""
        return cache_service.get_latest_metrics()
    
    @staticmethod
    def get_metrics_history(
        db: Session,
        start_time: Optional[datetime] = None,
        end_time: Optional[datetime] = None,
        page: int = 1,
        page_size: int = 100
    ) -> tuple[list[SystemMetrics], int]:
        """Get historical metrics with pagination."""
        query = db.query(SystemMetrics)
        
        if start_time:
            query = query.filter(SystemMetrics.timestamp >= start_time)
        if end_time:
            query = query.filter(SystemMetrics.timestamp <= end_time)
        
        # Get total count
        total = query.count()
        
        # Apply pagination
        offset = (page - 1) * page_size
        metrics = query.order_by(SystemMetrics.timestamp.desc()).offset(offset).limit(page_size).all()
        
        return metrics, total


# Singleton instance
metrics_service = MetricsService()
