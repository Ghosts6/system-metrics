# API Documentation

## Base URL

All API endpoints are prefixed with `/api/v1`.

```
http://localhost:8000/api/v1
```

## Health Endpoints

### GET /api/v1/health

Check service health status.

**Response:**
```json
{
  "status": "ok",
  "database": "ok",
  "redis": "ok"
}
```

### GET /api/v1/metrics/health

Check metrics service health.

**Response:**
```json
{
  "status": "ok",
  "redis": "ok"
}
```

## Metrics Endpoints

### GET /api/v1/metrics/live

Get the latest system metrics from cache. Returns cached data if available, otherwise collects fresh metrics.

**Response:**
```json
{
  "cpu_percent": 45.5,
  "cpu_count": 4,
  "cpu_freq_current": 2400.0,
  "memory_total": 8589934592,
  "memory_available": 4294967296,
  "memory_used": 4294967296,
  "memory_percent": 50.0,
  "disk_total": 107374182400,
  "disk_used": 53687091200,
  "disk_free": 53687091200,
  "disk_percent": 50.0,
  "network_bytes_sent": 1024000,
  "network_bytes_recv": 2048000,
  "hostname": "example-host",
  "platform": "Linux",
  "timestamp": "2024-01-15T10:30:00",
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
```

### POST /api/v1/metrics/collect

Collect current system metrics and save to database. Also updates the cache.

**Response:** `201 Created`
```json
{
  "id": 1,
  "timestamp": "2024-01-15T10:30:00",
  "cpu_percent": 45.5,
  "cpu_count": 4,
  "memory_percent": 50.0,
  "disk_percent": 50.0,
  ...
}
```

### GET /api/v1/metrics/history

Get historical system metrics with pagination and time filtering.

**Query Parameters:**
- `start_time` (optional): ISO 8601 datetime string
- `end_time` (optional): ISO 8601 datetime string
- `page` (default: 1): Page number (min: 1)
- `page_size` (default: 100): Items per page (min: 1, max: 1000)

**Example:**
```
GET /api/v1/metrics/history?page=1&page_size=10&start_time=2024-01-15T00:00:00
```

**Response:**
```json
{
  "items": [
    {
      "id": 1,
      "timestamp": "2024-01-15T10:30:00",
      "cpu_percent": 45.5,
      ...
    }
  ],
  "total": 100,
  "page": 1,
  "page_size": 10,
  "pages": 10
}
```

## Logs Endpoints

### POST /api/v1/logs/

Create a new log entry.

**Request Body:**
```json
{
  "level": "INFO",
  "message": "Application started",
  "source": "backend",
  "log_metadata": "{\"key\": \"value\"}"
}
```

**Fields:**
- `level` (required): Log level (INFO, WARNING, ERROR, etc.)
- `message` (required): Log message
- `source` (optional): Source identifier
- `log_metadata` (optional): Additional metadata as JSON string

**Response:** `201 Created`
```json
{
  "id": 1,
  "timestamp": "2024-01-15T10:30:00",
  "level": "INFO",
  "message": "Application started",
  "source": "backend",
  "log_metadata": "{\"key\": \"value\"}"
}
```

### GET /api/v1/logs/

Get system logs with filtering and pagination.

**Query Parameters:**
- `level` (optional): Filter by log level (INFO, WARNING, ERROR, etc.)
- `start_time` (optional): ISO 8601 datetime string
- `end_time` (optional): ISO 8601 datetime string
- `page` (default: 1): Page number (min: 1)
- `page_size` (default: 100): Items per page (min: 1, max: 1000)

**Example:**
```
GET /api/v1/logs/?level=ERROR&page=1&page_size=20
```

**Response:**
```json
{
  "items": [
    {
      "id": 1,
      "timestamp": "2024-01-15T10:30:00",
      "level": "ERROR",
      "message": "Database connection failed",
      "source": "backend"
    }
  ],
  "total": 50,
  "page": 1,
  "page_size": 20,
  "pages": 3
}
```

### GET /api/v1/logs/{log_id}

Get a specific log entry by ID.

**Response:**
```json
{
  "id": 1,
  "timestamp": "2024-01-15T10:30:00",
  "level": "INFO",
  "message": "Application started",
  "source": "backend",
  "log_metadata": null
}
```

**Error:** `404 Not Found` if log doesn't exist

## Error Responses

All endpoints may return the following error codes:

- `400 Bad Request`: Invalid request parameters
- `404 Not Found`: Resource not found
- `422 Unprocessable Entity`: Validation error
- `500 Internal Server Error`: Server error

**Error Response Format:**
```json
{
  "detail": "Error message description"
}
```

## Data Models

### SystemMetrics

- `cpu_percent`: CPU usage percentage (0-100)
- `cpu_count`: Number of CPU cores
- `cpu_freq_current`: Current CPU frequency in MHz
- `cpu_brand`: CPU brand name
- `cpu_vendor_id`: CPU vendor ID
- `memory_total`: Total memory in bytes
- `memory_available`: Available memory in bytes
- `memory_used`: Used memory in bytes
- `memory_percent`: Memory usage percentage (0-100)
- `disk_total`: Total disk space in bytes
- `disk_used`: Used disk space in bytes
- `disk_free`: Free disk space in bytes
- `disk_percent`: Disk usage percentage (0-100)
- `network_bytes_sent`: Network bytes sent
- `network_bytes_recv`: Network bytes received
- `hostname`: System hostname
- `platform`: Operating system platform
- `uptime_seconds`: System uptime in seconds
- `gpu_count`: Number of GPUs
- `gpus`: List of `GpuMetrics` objects (see below)

### GpuMetrics

- `name`: GPU name (e.g., "NVIDIA GeForce RTX 5070")
- `driver_version`: GPU driver version
- `memory_total`: Total GPU memory in bytes
- `memory_used`: Used GPU memory in bytes
- `temperature`: GPU temperature in Celsius
- `utilization`: GPU utilization percentage (0-100)

### SystemLog

- `level`: Log level (INFO, WARNING, ERROR, etc.)
- `message`: Log message text
- `source`: Source identifier
- `log_metadata`: Additional metadata as JSON string
- `timestamp`: ISO 8601 datetime string

## Interactive Documentation

Swagger UI documentation is available at:
```
http://localhost:8000/docs
```

ReDoc documentation is available at:
```
http://localhost:8000/redoc
```
