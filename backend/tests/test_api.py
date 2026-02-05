import pytest
from fastapi.testclient import TestClient
# from app.services.metrics_service import metrics_service # Removed as it's not used directly here


@pytest.fixture(scope="function")
def seeded_client(client: TestClient):
    """Fixture to ensure some metrics and logs exist in the DB for /live, /history and /logs endpoints."""
    metrics_data = get_test_metrics_data()
    # Seed at least one metric for /live
    client.post("/api/v1/metrics/collect", json=metrics_data)
    # Add more data for history
    for _ in range(5):
        client.post("/api/v1/metrics/collect", json=metrics_data)
    
    # Seed some logs
    for level in ["INFO", "WARNING", "ERROR"]:
        client.post("/api/v1/logs/", json={
            "level": level,
            "message": f"Test {level} message seeded by fixture"
        })
    client.post("/api/v1/logs/", json={"level": "INFO", "message": "Info message for filter test"})
    client.post("/api/v1/logs/", json={"level": "ERROR", "message": "Error message for filter test"})
    
    return client


def get_test_metrics_data():
    """Helper to generate valid SystemMetricsCreate data."""
    return {
        "cpu_percent": 10.5,
        "cpu_count": 4,
        "cpu_freq_current": 2500.0,
        "cpu_freq_min": 800.0,
        "cpu_freq_max": 3000.0,
        "memory_total": 8589934592,  # 8 GB
        "memory_available": 4294967296, # 4 GB
        "memory_used": 4294967296, # 4 GB
        "memory_percent": 50.0,
        "disk_total": 536870912000, # 500 GB
        "disk_used": 268435456000, # 250 GB
        "disk_free": 268435456000, # 250 GB
        "disk_percent": 50.0,
        "network_bytes_sent": 100000,
        "network_bytes_recv": 200000,
        "hostname": "test-host",
        "platform": "Linux",
        "uptime_seconds": 3600.0,
        "gpu_count": 1,
        "gpus": [
            {
                "name": "NVIDIA GeForce RTX 5070",
                "driver_version": "580.95.05",
                "memory_total": 12884901888,
                "memory_used": 546200064,
                "temperature": 36.0,
                "utilization": 1.0
            }
        ]
    }


def test_metrics_live_endpoint(seeded_client: TestClient):
    """Test live metrics endpoint."""
    response = seeded_client.get("/api/v1/metrics/live")
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
    assert "gpu_count" in data
    assert "gpus" in data
    
    # Verify data types and ranges
    assert isinstance(data["cpu_percent"], (int, float))
    assert 0 <= data["cpu_percent"] <= 100
    assert isinstance(data["memory_percent"], (int, float))
    assert 0 <= data["memory_percent"] <= 100
    assert isinstance(data["gpu_count"], int)
    assert data["gpu_count"] >= 0
    if data["gpu_count"] > 0:
        assert isinstance(data["gpus"], list)
        assert len(data["gpus"]) == data["gpu_count"]
        gpu = data["gpus"][0]
        assert "name" in gpu
        assert "utilization" in gpu
        assert isinstance(gpu["utilization"], (int, float))
        assert 0 <= gpu["utilization"] <= 100


def test_metrics_collect_endpoint(client: TestClient):
    """Test metrics collection endpoint."""
    metrics_data = get_test_metrics_data()
    response = client.post("/api/v1/metrics/collect", json=metrics_data)
    assert response.status_code == 201
    data = response.json()
    
    assert "id" in data
    assert "timestamp" in data
    assert "cpu_percent" in data
    assert "memory_percent" in data
    assert "disk_percent" in data
    assert "gpu_count" in data
    assert "gpus" in data
    
    if data["gpu_count"] > 0:
        assert isinstance(data["gpus"], list)
        assert len(data["gpus"]) == data["gpu_count"]
        gpu = data["gpus"][0]
        assert "name" in gpu
        assert "utilization" in gpu
        assert isinstance(gpu["utilization"], (int, float))
        assert 0 <= gpu["utilization"] <= 100

@pytest.mark.parametrize("invalid_payload, expected_loc, expected_msg", [
    # Missing required field: cpu_percent
    ({"cpu_count": 4, "memory_total": 1, "memory_available": 1, "memory_used": 1, "memory_percent": 1.0,
      "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0},
     ["body", "cpu_percent"], "Field required"),
    # Invalid data type: cpu_percent as string
    ({"cpu_percent": "abc", "cpu_count": 4, "memory_total": 1, "memory_available": 1, "memory_used": 1,
      "memory_percent": 1.0, "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0},
     ["body", "cpu_percent"], "Input should be a valid number, unable to parse string as a number"),
    # Out of range: cpu_percent > 100
    ({"cpu_percent": 101.0, "cpu_count": 4, "memory_total": 1, "memory_available": 1, "memory_used": 1,
      "memory_percent": 1.0, "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0},
     ["body", "cpu_percent"], "Input should be less than or equal to 100"),
    # Out of range: cpu_percent < 0
    ({"cpu_percent": -1.0, "cpu_count": 4, "memory_total": 1, "memory_available": 1, "memory_used": 1,
      "memory_percent": 1.0, "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0},
     ["body", "cpu_percent"], "Input should be greater than or equal to 0"),
    # Invalid data type: cpu_count as float
    ({"cpu_percent": 10.0, "cpu_count": 4.5, "memory_total": 1, "memory_available": 1, "memory_used": 1,
      "memory_percent": 1.0, "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0},
     ["body", "cpu_count"], "Input should be a valid integer, got a number with a fractional part"),
    # Out of range: cpu_count <= 0
    ({"cpu_percent": 10.0, "cpu_count": 0, "memory_total": 1, "memory_available": 1, "memory_used": 1,
      "memory_percent": 1.0, "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0},
     ["body", "cpu_count"], "Input should be greater than 0"),
    # Missing required field within GPU metrics: name
    ({"cpu_percent": 10.0, "cpu_count": 4, "memory_total": 1, "memory_available": 1, "memory_used": 1,
      "memory_percent": 1.0, "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0,
      "gpu_count": 1, "gpus": [{"driver_version": "1.0", "memory_total": 1, "memory_used": 1,
                                "temperature": 30.0, "utilization": 50.0}]},
     ["body", "gpus", 0, "name"], "Field required"),
    # Out of range: GPU utilization > 100
    ({"cpu_percent": 10.0, "cpu_count": 4, "memory_total": 1, "memory_available": 1, "memory_used": 1,
      "memory_percent": 1.0, "disk_total": 1, "disk_used": 1, "disk_free": 1, "disk_percent": 1.0,
      "gpu_count": 1, "gpus": [{"name": "GPU", "driver_version": "1.0", "memory_total": 1, "memory_used": 1,
                                "temperature": 30.0, "utilization": 101.0}]},
     ["body", "gpus", 0, "utilization"], "Input should be less than or equal to 100"),
])
def test_metrics_collect_endpoint_invalid_input(client: TestClient, invalid_payload: dict, expected_loc: list, expected_msg: str):
    """Test metrics collection endpoint with invalid input data."""
    response = client.post("/api/v1/metrics/collect", json=invalid_payload)
    assert response.status_code == 422
    
    details = response.json()["detail"]
    assert any(
        err.get("loc") == expected_loc and expected_msg in err.get("msg")
        for err in details
    ), f"Expected error not found in details: {details}"


@pytest.mark.parametrize("invalid_payload, expected_loc, expected_msg", [
    # Missing required field: message
    ({"level": "INFO"}, ["body", "message"], "Field required"),
    # Missing required field: level
    ({"message": "Test message"}, ["body", "level"], "Field required"),
])
def test_logs_create_endpoint_invalid_input(client: TestClient, invalid_payload: dict, expected_loc: list, expected_msg: str):
    """Test log creation endpoint with invalid input data."""
    response = client.post("/api/v1/logs/", json=invalid_payload)
    assert response.status_code == 422
    
    details = response.json()["detail"]
    assert any(
        err.get("loc") == expected_loc and expected_msg in err.get("msg")
        for err in details
    ), f"Expected error not found in details: {details}"


def test_metrics_history_endpoint(seeded_client: TestClient):
    """Test metrics history endpoint."""
    # Get history
    response = seeded_client.get("/api/v1/metrics/history?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    
    assert "items" in data
    assert "total" in data
    assert "page" in data
    assert "page_size" in data
    assert "pages" in data
    assert isinstance(data["items"], list)
    assert data["total"] >= 2 # Should have at least one from fixture + one from test


def test_metrics_history_pagination(seeded_client: TestClient):
    """Test metrics history pagination."""
    # Test first page
    response = seeded_client.get("/api/v1/metrics/history?page=1&page_size=2")
    assert response.status_code == 200
    data = response.json()
    assert len(data["items"]) == 2
    assert data["page"] == 1
    assert data["page_size"] == 2
    
    # Test second page
    response = seeded_client.get("/api/v1/metrics/history?page=2&page_size=2")
    assert response.status_code == 200
    data = response.json()
    assert len(data["items"]) == 2
    assert data["page"] == 2


def test_metrics_history_invalid_pagination(seeded_client: TestClient):
    """Test metrics history with invalid pagination parameters."""
    
    # Invalid page (less than 1)
    response = seeded_client.get("/api/v1/metrics/history?page=0&page_size=10")
    assert response.status_code == 422
    details = response.json()["detail"]
    assert any(
        err.get("loc") == ["query", "page"] and "greater than or equal to 1" in err.get("msg")
        for err in details
    ), f"Expected error for page=0 not found in details: {details}"

    # Invalid page_size (too large)
    response = seeded_client.get("/api/v1/metrics/history?page=1&page_size=10000")
    assert response.status_code == 422
    details = response.json()["detail"]
    assert any(
        err.get("loc") == ["query", "page_size"] and "less than or equal to 100" in err.get("msg")
        for err in details
    ), f"Expected error for page_size=10000 not found in details: {details}"

    # Invalid page (non-integer)
    response = seeded_client.get("/api/v1/metrics/history?page=abc&page_size=10")
    assert response.status_code == 422
    details = response.json()["detail"]
    assert any(
        err.get("loc") == ["query", "page"] and "Input should be a valid integer" in err.get("msg")
        for err in details
    ), f"Expected error for page='abc' not found in details: {details}"

    # Invalid page_size (non-integer)
    response = seeded_client.get("/api/v1/metrics/history?page=1&page_size=xyz")
    assert response.status_code == 422
    details = response.json()["detail"]
    assert any(
        err.get("loc") == ["query", "page_size"] and "Input should be a valid integer" in err.get("msg")
        for err in details
    ), f"Expected error for page_size='xyz' not found in details: {details}"


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


def test_logs_get_endpoint(seeded_client: TestClient):
    """Test logs retrieval endpoint."""
    # Get all logs
    response = seeded_client.get("/api/v1/logs/?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    
    assert "items" in data
    assert "total" in data
    assert len(data["items"]) >= 3 # At least 3 seeded logs


def test_logs_filter_by_level(seeded_client: TestClient):
    """Test filtering logs by level."""
    # Filter by ERROR level
    response = seeded_client.get("/api/v1/logs/?level=ERROR&page=1&page_size=10")
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


def test_logs_get_nonexistent_id(seeded_client: TestClient):
    """Test getting a non-existent log ID."""
    response = seeded_client.get("/api/v1/logs/99999")
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
