import pytest
from fastapi.testclient import TestClient
from app.main import app
from tests.test_api import seeded_client # Import seeded_client fixture

@pytest.fixture
def simple_client():
    return TestClient(app)


def test_read_root(simple_client):
    response = simple_client.get("/")
    assert response.status_code == 200
    data = response.json()
    assert data["status"] == "ok"
    assert "message" in data
    assert "version" in data


def test_read_health(simple_client):
    response = simple_client.get("/api/v1/health")
    assert response.status_code == 200
    data = response.json()
    assert data["status"] == "ok"
    # assert "redis" in data # No longer checked in router/metrics.py


def test_get_live_metrics(seeded_client: TestClient):
    response = seeded_client.get("/api/v1/metrics/live")
    assert response.status_code == 200
    data = response.json()
    assert "cpu_percent" in data
    assert "memory_percent" in data
    assert "disk_percent" in data
    assert "gpu_count" in data
    assert "gpus" in data
    
    # Verify GPU data if present
    if data["gpu_count"] > 0:
        assert isinstance(data["gpus"], list)
        assert len(data["gpus"]) == data["gpu_count"]
        gpu = data["gpus"][0]
        assert "name" in gpu
        assert "utilization" in gpu
        assert isinstance(gpu["utilization"], (int, float))
        assert 0 <= gpu["utilization"] <= 100

def test_collect_metrics(client: TestClient):
    # This test should use a fresh client to test the collect endpoint specifically
    metrics_data = { # Minimal data for this test, as get_test_metrics_data has more
        "cpu_percent": 10.0, "cpu_count": 1, 
        "memory_total": 100, "memory_available": 50, "memory_used": 50, "memory_percent": 50.0,
        "disk_total": 100, "disk_used": 50, "disk_free": 50, "disk_percent": 50.0,
    }
    response = client.post("/api/v1/metrics/collect", json=metrics_data)
    assert response.status_code == 201
    data = response.json()
    assert "id" in data
    assert "timestamp" in data
    assert "cpu_percent" in data


def test_get_metrics_history(seeded_client: TestClient):
    response = seeded_client.get("/api/v1/metrics/history?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    assert "items" in data
    assert "total" in data
    assert "page" in data
    assert "page_size" in data


def test_create_log(client: TestClient):
    log_data = {
        "level": "INFO",
        "message": "Test log message",
        "source": "test"
    }
    response = client.post("/api/v1/logs/", json=log_data)
    assert response.status_code == 201
    data = response.json()
    assert data["level"] == "INFO"
    assert data["message"] == "Test log message"


def test_get_logs(seeded_client: TestClient):
    response = seeded_client.get("/api/v1/logs/?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    assert "items" in data
    assert "total" in data