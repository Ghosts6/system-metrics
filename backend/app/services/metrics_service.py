import psutil
import socket
import platform
import json
import subprocess
import os
from datetime import datetime
from typing import Dict, Any, Optional
from app.schemas import SystemMetricsCreate
from app.models import SystemMetrics
from app.services.cache_service import cache_service
from app.config import settings
from sqlalchemy.orm import Session


class MetricsService:
    """Service for collecting and managing system metrics."""
    
    @staticmethod
    def _collect_with_psutil() -> SystemMetricsCreate:
        """Collect metrics using psutil (Python)."""
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
        
        # Calculate uptime in seconds
        boot_time_timestamp = psutil.boot_time()
        uptime_seconds = datetime.now().timestamp() - boot_time_timestamp
        
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
            platform=platform_name,
            uptime_seconds=uptime_seconds
        )
    
    @staticmethod
    def _collect_with_c_collector() -> Optional[SystemMetricsCreate]:
        """Collect metrics using C collector binary."""
        collector_path = settings.c_collector_path
        
        if not collector_path:
            default_paths = [
                "./collector-c/collector",
                "../collector-c/collector",
                "/usr/local/bin/collector",
                "collector"
            ]
            for path in default_paths:
                if os.path.isfile(path) and os.access(path, os.X_OK):
                    collector_path = path
                    break
        
        if not collector_path or not os.path.isfile(collector_path):
            return None
        
        try:
            result = subprocess.run(
                [collector_path, "--output"],
                capture_output=True,
                text=True,
                timeout=5,
                check=True
            )
            
            metrics_json = json.loads(result.stdout.strip())
            
            return SystemMetricsCreate(
                cpu_percent=float(metrics_json.get("cpu_percent", 0)),
                cpu_count=int(metrics_json.get("cpu_count", 0)),
                cpu_freq_current=float(metrics_json.get("cpu_freq_current")) if metrics_json.get("cpu_freq_current") else None,
                cpu_freq_min=float(metrics_json.get("cpu_freq_min")) if metrics_json.get("cpu_freq_min") else None,
                cpu_freq_max=float(metrics_json.get("cpu_freq_max")) if metrics_json.get("cpu_freq_max") else None,
                memory_total=int(metrics_json.get("memory_total", 0)),
                memory_available=int(metrics_json.get("memory_available", 0)),
                memory_used=int(metrics_json.get("memory_used", 0)),
                memory_percent=float(metrics_json.get("memory_percent", 0)),
                disk_total=int(metrics_json.get("disk_total", 0)),
                disk_used=int(metrics_json.get("disk_used", 0)),
                disk_free=int(metrics_json.get("disk_free", 0)),
                disk_percent=float(metrics_json.get("disk_percent", 0)),
                network_bytes_sent=int(metrics_json.get("network_bytes_sent")) if metrics_json.get("network_bytes_sent") else None,
                network_bytes_recv=int(metrics_json.get("network_bytes_recv")) if metrics_json.get("network_bytes_recv") else None,
                hostname=metrics_json.get("hostname"),
                platform=metrics_json.get("platform")
            )
        except (subprocess.TimeoutExpired, subprocess.CalledProcessError, json.JSONDecodeError, KeyError, ValueError) as e:
            return None
    
    @staticmethod
    def collect_metrics() -> SystemMetricsCreate:
        """Collect current system metrics using configured collector."""
        if settings.metrics_collector == "c-collector":
            metrics = MetricsService._collect_with_c_collector()
            if metrics:
                return metrics
            else:
                # Fallback to psutil if C collector fails
                return MetricsService._collect_with_psutil()
        else:
            # Default to psutil
            return MetricsService._collect_with_psutil()
    
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
