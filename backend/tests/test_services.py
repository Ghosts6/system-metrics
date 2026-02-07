import pytest
import os
import json
import subprocess
from unittest.mock import patch, MagicMock
from sqlalchemy.orm import Session
from app.services.metrics_service import metrics_service
from app.services.cache_service import cache_service
from app.schemas import SystemMetricsCreate, GpuMetricsBase

def test_collect_metrics():
    """Test metrics collection."""
    metrics = metrics_service.collect_metrics()
    
    assert isinstance(metrics, SystemMetricsCreate)
    assert metrics.cpu_percent >= 0
    assert metrics.cpu_percent <= 100
    assert metrics.cpu_count > 0
    assert metrics.memory_total > 0
    assert metrics.memory_percent >= 0
    assert metrics.memory_percent <= 100
    assert metrics.disk_total > 0
    assert metrics.disk_percent >= 0
    assert metrics.disk_percent <= 100

def test_cache_service_connection():
    """Test cache service connection check."""
    is_connected = cache_service.is_connected()
    assert isinstance(is_connected, bool)

def test_cache_service_stats():
    """Test cache service statistics."""
    stats = cache_service.get_cache_stats()
    assert isinstance(stats, dict)
    assert "connected" in stats

def test_collect_with_psutil():
    """Test psutil-based metrics collection."""
    metrics = metrics_service._collect_with_psutil()
    
    assert isinstance(metrics, SystemMetricsCreate)
    assert metrics.cpu_percent >= 0
    assert metrics.cpu_percent <= 100
    assert metrics.cpu_count > 0
    assert metrics.memory_total > 0
    assert metrics.memory_percent >= 0
    assert metrics.memory_percent <= 100

def test_collect_with_c_collector_success():
    """Test C collector integration with successful execution."""
    mock_json = {
        "cpu_percent": 45.5,
        "cpu_count": 4,
        "memory_total": 8589934592,
        "memory_available": 4294967296,
        "memory_used": 4294967296,
        "memory_percent": 50.0,
        "disk_total": 107374182400,
        "disk_used": 53687091200,
        "disk_free": 53687091200,
        "disk_percent": 50.0,
        "hostname": "test-host",
        "platform": "Linux"
    }
    
    mock_result = MagicMock()
    mock_result.stdout = json.dumps(mock_json)
    mock_result.returncode = 0
    
    with patch('subprocess.run', return_value=mock_result), \
         patch('os.path.isfile', return_value=True), \
         patch('os.access', return_value=True):
        metrics = metrics_service._collect_with_c_collector()
    
    if metrics:
        assert isinstance(metrics, SystemMetricsCreate)
        assert metrics.cpu_percent == 45.5
        assert metrics.cpu_count == 4
        assert metrics.memory_percent == 50.0
        assert metrics.hostname == "test-host"

def test_collect_with_c_collector_not_found(caplog):
    """Test C collector fallback when binary not found."""
    with patch('os.path.isfile', return_value=False):
        metrics = metrics_service._collect_with_c_collector()
        assert metrics is None
        assert "C collector binary not found or not executable" in caplog.text

def test_collect_with_c_collector_invalid_json(caplog):
    """Test C collector handling of invalid JSON."""
    mock_result = MagicMock()
    mock_result.stdout = "invalid json"
    mock_result.returncode = 0
    
    with patch('subprocess.run', return_value=mock_result), \
         patch('os.path.isfile', return_value=True), \
         patch('os.access', return_value=True):
        metrics = metrics_service._collect_with_c_collector()
        assert metrics is None
        assert "Failed to parse C collector output" in caplog.text

def test_collect_with_c_collector_subprocess_error(caplog):
    """Test C collector handling of subprocess errors."""
    with patch('subprocess.run', side_effect=subprocess.CalledProcessError(1, "collector")):
        with patch('os.path.isfile', return_value=True):
            with patch('os.access', return_value=True):
                metrics = metrics_service._collect_with_c_collector()
                assert metrics is None
                assert "C collector failed with exit code 1" in caplog.text

def test_collect_metrics_with_c_collector_config():
    """Test collect_metrics using C collector when configured."""
    mock_json = {
        "cpu_percent": 30.0,
        "cpu_count": 8,
        "memory_total": 17179869184,
        "memory_available": 8589934592,
        "memory_used": 8589934592,
        "memory_percent": 50.0,
        "disk_total": 107374182400,
        "disk_used": 53687091200,
        "disk_free": 53687091200,
        "disk_percent": 50.0,
        "hostname": "test",
        "platform": "Linux"
    }
    
    mock_result = MagicMock()
    mock_result.stdout = json.dumps(mock_json)
    mock_result.returncode = 0
    
    with patch('app.services.metrics_service.settings.metrics_collector', "c-collector"), \
         patch('app.services.metrics_service.settings.c_collector_path', None), \
         patch('subprocess.run', return_value=mock_result), \
         patch('os.path.isfile', return_value=True), \
         patch('os.access', return_value=True):
        metrics = metrics_service.collect_metrics()
        
        assert isinstance(metrics, SystemMetricsCreate)
        assert metrics.cpu_percent == 30.0
        assert metrics.cpu_count == 8

def test_collect_metrics_fallback_to_psutil(caplog):
    """Test collect_metrics falls back to psutil when C collector fails."""
    with patch('app.services.metrics_service.settings.metrics_collector', "c-collector"), \
         patch('app.services.metrics_service.settings.c_collector_path', None), \
         patch('os.path.isfile', return_value=False):
        metrics = metrics_service.collect_metrics()
        
        assert isinstance(metrics, SystemMetricsCreate)
        assert metrics.cpu_count > 0
        assert metrics.memory_total > 0
        assert "C collector failed. Falling back to psutil." in caplog.text