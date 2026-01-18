import pytest
from sqlalchemy.orm import Session
from app.services.metrics_service import metrics_service
from app.services.cache_service import cache_service
from app.schemas import SystemMetricsCreate


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


def test_save_metrics(db_session: Session):
    """Test saving metrics to database."""
    metrics = metrics_service.collect_metrics()
    saved_metrics = metrics_service.save_metrics(db_session, metrics)
    
    assert saved_metrics.id is not None
    assert saved_metrics.cpu_percent == metrics.cpu_percent
    assert saved_metrics.memory_percent == metrics.memory_percent
    assert saved_metrics.timestamp is not None


def test_cache_latest_metrics():
    """Test caching latest metrics."""
    metrics = metrics_service.collect_metrics()
    result = metrics_service.cache_latest_metrics(metrics)
    
    # Cache may or may not be available in test environment
    # Just verify the function doesn't crash
    assert isinstance(result, bool)


def test_get_latest_metrics():
    """Test retrieving latest metrics from cache."""
    # This may return None if Redis is not available
    cached = metrics_service.get_latest_metrics()
    # Just verify it doesn't crash
    assert cached is None or isinstance(cached, dict)


def test_get_metrics_history(db_session: Session):
    """Test retrieving metrics history."""
    # Create some test metrics
    for i in range(10):
        metrics = metrics_service.collect_metrics()
        metrics_service.save_metrics(db_session, metrics)
    
    # Get history with pagination
    history, total = metrics_service.get_metrics_history(
        db=db_session,
        page=1,
        page_size=5
    )
    
    assert len(history) == 5
    assert total == 10
    
    # Test pagination
    history_page2, total2 = metrics_service.get_metrics_history(
        db=db_session,
        page=2,
        page_size=5
    )
    
    assert len(history_page2) == 5
    assert total2 == 10
    # Verify different items
    assert history[0].id != history_page2[0].id


def test_get_metrics_history_with_time_filter(db_session: Session):
    """Test metrics history with time filtering."""
    from datetime import datetime, timedelta
    
    # Create metrics
    metrics = metrics_service.collect_metrics()
    metrics_service.save_metrics(db_session, metrics)
    
    # Get history with time filter
    end_time = datetime.utcnow()
    start_time = end_time - timedelta(hours=1)
    
    history, total = metrics_service.get_metrics_history(
        db=db_session,
        start_time=start_time,
        end_time=end_time,
        page=1,
        page_size=10
    )
    
    assert total >= 1
    assert len(history) >= 1


def test_cache_service_connection():
    """Test cache service connection check."""
    # This may fail if Redis is not available, which is OK for tests
    is_connected = cache_service.is_connected()
    assert isinstance(is_connected, bool)


def test_cache_service_stats():
    """Test cache service statistics."""
    stats = cache_service.get_cache_stats()
    assert isinstance(stats, dict)
    assert "connected" in stats
