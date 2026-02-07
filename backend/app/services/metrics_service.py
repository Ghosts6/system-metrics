import psutil
import socket
import platform
import json
import subprocess
import os
from datetime import datetime
from typing import Dict, Any, Optional
from loguru import logger
from app.schemas import SystemMetricsCreate, GpuMetricsBase
from app.models import SystemMetrics
from app.services.cache_service import cache_service
from app.config import settings
from sqlalchemy.orm import Session


class MetricsService:
    """Service for collecting and managing system metrics."""
    
    @staticmethod
    def _collect_with_psutil() -> SystemMetricsCreate:
        """Collect metrics using psutil (Python)."""
        logger.info("Collecting metrics with psutil.")
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
        logger.info("Attempting to collect metrics with C collector.")
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
            logger.warning("C collector binary not found or not executable.")
            return None
            
        logger.debug(f"Using C collector at: {collector_path}")
        
        try:
            result = subprocess.run(
                [collector_path, "--output"],
                capture_output=True,
                text=True,
                timeout=5,
                check=True
            )
            
            metrics_json = json.loads(result.stdout.strip())
            
            # Parse GPU metrics
            gpu_metrics_list = []
            if "gpus" in metrics_json:
                for gpu_data in metrics_json["gpus"]:
                    gpu_metrics_list.append(GpuMetricsBase(
                        name=gpu_data.get("name", "Unknown GPU"),
                        driver_version=gpu_data.get("driver_version", "N/A"),
                        memory_total=int(gpu_data.get("memory_total", 0)),
                        memory_used=int(gpu_data.get("memory_used", 0)),
                        temperature=float(gpu_data.get("temperature", 0.0)),
                        utilization=float(gpu_data.get("utilization", 0.0))
                    ))

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
                platform=metrics_json.get("platform"),
                uptime_seconds=float(metrics_json.get("uptime_seconds", 0.0)),
                gpu_count=int(metrics_json.get("gpu_count", 0)),
                gpus=gpu_metrics_list
            )
        except subprocess.TimeoutExpired:
            logger.error("C collector timed out.", exc_info=True)
            return None
        except subprocess.CalledProcessError as e:
            logger.error(f"C collector failed with exit code {e.returncode}: {e.stderr}", exc_info=True)
            return None
        except (json.JSONDecodeError, KeyError, ValueError) as e:
            logger.error(f"Failed to parse C collector output: {e}", exc_info=True)
            return None
    
    @staticmethod
    def collect_metrics() -> SystemMetricsCreate:
        """Collect current system metrics using configured collector."""
        if settings.metrics_collector == "c-collector":
            metrics = MetricsService._collect_with_c_collector()
            if metrics:
                logger.info("Successfully collected metrics with C collector.")
                return metrics
            else:
                logger.warning("C collector failed. Falling back to psutil.")
                return MetricsService._collect_with_psutil()
        else:
            return MetricsService._collect_with_psutil()


# Singleton instance
metrics_service = MetricsService()
