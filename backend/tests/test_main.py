import pytest
from fastapi.testclient import TestClient
from app.main import app


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
    assert "database" in data
    assert "redis" in data


def test_get_live_metrics(simple_client):
    response = simple_client.get("/api/v1/metrics/live")
    assert response.status_code == 200
    data = response.json()
    assert "cpu_percent" in data
    assert "memory_percent" in data
    assert "disk_percent" in data


def test_collect_metrics(client):
    response = client.post("/api/v1/metrics/collect")
    assert response.status_code == 201
    data = response.json()
    assert "id" in data
    assert "timestamp" in data
    assert "cpu_percent" in data


def test_get_metrics_history(client):
    response = client.get("/api/v1/metrics/history?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    assert "items" in data
    assert "total" in data
    assert "page" in data
    assert "page_size" in data


def test_create_log(client):
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


def test_get_logs(client):
    response = client.get("/api/v1/logs/?page=1&page_size=10")
    assert response.status_code == 200
    data = response.json()
    assert "items" in data
    assert "total" in data
