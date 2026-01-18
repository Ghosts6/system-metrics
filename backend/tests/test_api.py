import pytest
from fastapi.testclient import TestClient
from app.services.metrics_service import metrics_service


def test_metrics_live_endpoint(client: TestClient):
    """Test live metrics endpoint."""
    response = client.get("/api/v1/metrics/live")
    assert response.status_code == 200
    data = response.json()
    
    # Verify all expected fields
    assert "cpu_percent" in data
    assert "cpu_count" in data
    assert "memory_total" in data
    assert "memory_percent" in data
    assert "disk_total" in data
    assert "disk_percent" in data
    assert "timestamp" in data
    
    # Verify data types and ranges
    assert isinstance(data["cpu_percent"], (int, float))
    assert 0 <= data["cpu_percent"] <= 100
    assert isinstance(data["memory_percent"], (int, float))
    assert 0 <= data["memory_percent"] <= 100


def test_metrics_collect_endpoint(client: TestClient):
    """Test metrics collection endpoint."""
    response = client.post("/api/v1/metrics/collect")
    assert response.status_code == 201
    data = response.json()
    
    assert "id" in data
    assert "timestamp" in data
    assert "cpu_percent" in data
    assert "memory_percent" in data
    assert "disk_percent" in data


def test_metrics_history_endpoint(client: TestClient):
    """Test metrics history endpoint."""
    # First, create some metrics
    client.post("/api/v1/metrics/collect")
    client.post("/api/v1/metrics/collect")
    
    # Get history
    response = client.get("/api/v1/metrics/history?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    
    assert "items" in data
    assert "total" in data
    assert "page" in data
    assert "page_size" in data
    assert "pages" in data
    assert isinstance(data["items"], list)
    assert data["total"] >= 2


def test_metrics_history_pagination(client: TestClient):
    """Test metrics history pagination."""
    # Create multiple metrics
    for _ in range(5):
        client.post("/api/v1/metrics/collect")
    
    # Test first page
    response = client.get("/api/v1/metrics/history?page=1&page_size=2")
    assert response.status_code == 200
    data = response.json()
    assert len(data["items"]) == 2
    assert data["page"] == 1
    assert data["page_size"] == 2
    
    # Test second page
    response = client.get("/api/v1/metrics/history?page=2&page_size=2")
    assert response.status_code == 200
    data = response.json()
    assert len(data["items"]) == 2
    assert data["page"] == 2


def test_metrics_history_invalid_pagination(client: TestClient):
    """Test metrics history with invalid pagination parameters."""
    # Invalid page (should default or error)
    response = client.get("/api/v1/metrics/history?page=0&page_size=10")
    # Should either return 422 (validation error) or handle gracefully
    assert response.status_code in [200, 422]
    
    # Invalid page_size (too large)
    response = client.get("/api/v1/metrics/history?page=1&page_size=10000")
    # Should either return 422 or cap at max
    assert response.status_code in [200, 422]


def test_logs_create_endpoint(client: TestClient):
    """Test log creation endpoint."""
    log_data = {
        "level": "INFO",
        "message": "Test log message",
        "source": "test_source",
        "log_metadata": '{"key": "value"}'
    }
    
    response = client.post("/api/v1/logs/", json=log_data)
    assert response.status_code == 201
    data = response.json()
    
    assert data["level"] == "INFO"
    assert data["message"] == "Test log message"
    assert data["source"] == "test_source"
    assert "id" in data
    assert "timestamp" in data


def test_logs_create_minimal(client: TestClient):
    """Test log creation with minimal required fields."""
    log_data = {
        "level": "WARNING",
        "message": "Minimal log message"
    }
    
    response = client.post("/api/v1/logs/", json=log_data)
    assert response.status_code == 201
    data = response.json()
    
    assert data["level"] == "WARNING"
    assert data["message"] == "Minimal log message"


def test_logs_get_endpoint(client: TestClient):
    """Test logs retrieval endpoint."""
    # Create some logs
    for level in ["INFO", "WARNING", "ERROR"]:
        client.post("/api/v1/logs/", json={
            "level": level,
            "message": f"Test {level} message"
        })
    
    # Get all logs
    response = client.get("/api/v1/logs/?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    
    assert "items" in data
    assert "total" in data
    assert len(data["items"]) >= 3


def test_logs_filter_by_level(client: TestClient):
    """Test filtering logs by level."""
    # Create logs with different levels
    client.post("/api/v1/logs/", json={"level": "INFO", "message": "Info message"})
    client.post("/api/v1/logs/", json={"level": "ERROR", "message": "Error message"})
    
    # Filter by ERROR level
    response = client.get("/api/v1/logs/?level=ERROR&page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    
    assert all(log["level"] == "ERROR" for log in data["items"])


def test_logs_get_by_id(client: TestClient):
    """Test getting a specific log by ID."""
    # Create a log
    response = client.post("/api/v1/logs/", json={
        "level": "INFO",
        "message": "Test message for ID retrieval"
    })
    assert response.status_code == 201
    log_id = response.json()["id"]
    
    # Get by ID
    response = client.get(f"/api/v1/logs/{log_id}")
    assert response.status_code == 200
    data = response.json()
    
    assert data["id"] == log_id
    assert data["message"] == "Test message for ID retrieval"


def test_logs_get_nonexistent_id(client: TestClient):
    """Test getting a non-existent log ID."""
    response = client.get("/api/v1/logs/99999")
    assert response.status_code == 404


def test_metrics_health_endpoint(client: TestClient):
    """Test metrics health endpoint."""
    response = client.get("/api/v1/metrics/health")
    assert response.status_code == 200
    data = response.json()
    
    assert "status" in data
    assert "redis" in data


def test_api_error_handling(client: TestClient):
    """Test API error handling."""
    # Test invalid endpoint
    response = client.get("/api/v1/invalid")
    assert response.status_code == 404
    
    # Test invalid method
    response = client.get("/api/v1/metrics/collect")  # Should be POST
    assert response.status_code == 405  # Method not allowed
