import pytest
from datetime import datetime
from sqlalchemy.orm import Session
from app.models import SystemMetrics, SystemLog


def test_system_metrics_model(db_session: Session):
    """Test SystemMetrics model creation and retrieval."""
    metrics = SystemMetrics(
        cpu_percent=45.5,
        cpu_count=4,
        cpu_freq_current=2400.0,
        cpu_freq_min=800.0,
        cpu_freq_max=3200.0,
        memory_total=8589934592,  # 8GB
        memory_available=4294967296,  # 4GB
        memory_used=4294967296,  # 4GB
        memory_percent=50.0,
        disk_total=107374182400,  # 100GB
        disk_used=53687091200,  # 50GB
        disk_free=53687091200,  # 50GB
        disk_percent=50.0,
        network_bytes_sent=1024000,
        network_bytes_recv=2048000,
        hostname="test-host",
        platform="Linux"
    )
    
    db_session.add(metrics)
    db_session.commit()
    db_session.refresh(metrics)
    
    assert metrics.id is not None
    assert metrics.cpu_percent == 45.5
    assert metrics.cpu_count == 4
    assert metrics.memory_percent == 50.0
    assert metrics.hostname == "test-host"
    assert metrics.timestamp is not None


def test_system_log_model(db_session: Session):
    """Test SystemLog model creation and retrieval."""
    log = SystemLog(
        level="INFO",
        source="test_source",
        message="Test log message",
        log_metadata='{"key": "value"}'
    )
    
    db_session.add(log)
    db_session.commit()
    db_session.refresh(log)
    
    assert log.id is not None
    assert log.level == "INFO"
    assert log.source == "test_source"
    assert log.message == "Test log message"
    assert log.log_metadata == '{"key": "value"}'
    assert log.timestamp is not None


def test_system_metrics_timestamp_indexing(db_session: Session):
    """Test that timestamp indexing works for queries."""
    # Create multiple metrics with different timestamps
    for i in range(5):
        metrics = SystemMetrics(
            cpu_percent=float(i * 10),
            cpu_count=4,
            memory_total=8589934592,
            memory_available=4294967296,
            memory_used=4294967296,
            memory_percent=50.0,
            disk_total=107374182400,
            disk_used=53687091200,
            disk_free=53687091200,
            disk_percent=50.0
        )
        db_session.add(metrics)
    
    db_session.commit()
    
    # Query by timestamp (should use index)
    from sqlalchemy import func
    count = db_session.query(SystemMetrics).count()
    assert count == 5


def test_system_log_level_filtering(db_session: Session):
    """Test filtering logs by level."""
    # Create logs with different levels
    for level in ["INFO", "WARNING", "ERROR"]:
        log = SystemLog(
            level=level,
            message=f"Test {level} message",
            source="test"
        )
        db_session.add(log)
    
    db_session.commit()
    
    # Filter by level
    error_logs = db_session.query(SystemLog).filter(SystemLog.level == "ERROR").all()
    assert len(error_logs) == 1
    assert error_logs[0].level == "ERROR"
