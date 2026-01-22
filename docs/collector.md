# C Metrics Collector

Low-level C agent for collecting system metrics and sending them to the backend API.

## Features

- Cross-platform support (Linux, macOS, Windows)
- Low-level system calls for accurate metrics
- HTTP POST to backend API
- JSON output mode for piping/scripting
- Configurable collection interval
- Integration with FastAPI backend

## Building

### Prerequisites

- GCC compiler (or MSVC on Windows)
- libcurl development libraries
- Make (or CMake on Windows)

**Linux:**
```bash
sudo apt-get install build-essential libcurl4-openssl-dev
```

**macOS:**
```bash
brew install curl
```

**Windows:**
- Install MinGW-w64 or Visual Studio
- Download libcurl from https://curl.se/windows/
- Or use vcpkg: `vcpkg install curl`

### Compile

**Linux/macOS:**
```bash
cd collector-c
make
```

**Windows (MinGW):**
```bash
cd collector-c
mingw32-make -f Makefile.win
```

This creates the `collector` executable (or `collector.exe` on Windows).

### Install

**Linux/macOS:**
```bash
sudo make install
```

Installs to `/usr/local/bin/collector`.

## Usage

### Standalone Mode - Send metrics to API

Run the collector as a standalone process that sends metrics via HTTP:

```bash
./collector -u http://localhost:8000 -i 5
```

- `-u, --url`: API base URL (default: http://localhost:8000)
- `-i, --interval`: Collection interval in seconds (default: 5)

### Output JSON to stdout

```bash
./collector --output
```

Useful for piping to other tools, testing, or backend integration.

### Examples

```bash
# Collect and send every 10 seconds
./collector -u http://api.example.com:8000 -i 10

# Output JSON once and exit
./collector --output

# Pipe JSON to a file
./collector --output > metrics.json
```

## Collected Metrics

- CPU: percentage, count, frequency (current/min/max)
- Memory: total, available, used, percentage
- Disk: total, used, free, percentage (root partition)
- Network: bytes sent/received (aggregated)
- System: hostname, platform

## Backend Integration

The FastAPI backend can use either psutil (Python) or the C collector for metrics collection.

### Configuration

Set the collector type via environment variable:

```bash
# Use psutil (default)
METRICS_COLLECTOR=psutil

# Use C collector
METRICS_COLLECTOR=c-collector
C_COLLECTOR_PATH=/path/to/collector
```

If `C_COLLECTOR_PATH` is not set, the backend will search for the collector in:
1. `./collector-c/collector` (relative to backend)
2. `../collector-c/collector` (parent directory)
3. `/usr/local/bin/collector` (system path)
4. `collector` (in PATH)

### How It Works

When `METRICS_COLLECTOR=c-collector`:

1. Backend calls the C collector binary with `--output` flag
2. C collector outputs JSON to stdout
3. Backend parses JSON and converts to `SystemMetricsCreate`
4. Falls back to psutil if C collector fails or is not found

### Benefits

- **Performance**: C collector uses low-level system calls, potentially faster
- **Accuracy**: Direct system API access for more precise metrics
- **Flexibility**: Can use either collector based on environment
- **Fallback**: Automatically falls back to psutil if C collector unavailable

### Usage Examples

**Docker Compose:**

Add to `docker.env`:
```env
METRICS_COLLECTOR=c-collector
C_COLLECTOR_PATH=/app/collector-c/collector
```

**Local Development:**
```bash
export METRICS_COLLECTOR=c-collector
export C_COLLECTOR_PATH=../collector-c/collector
uvicorn app.main:app --reload
```

**Standalone C Collector:**

The C collector can also run independently and send metrics via HTTP:
```bash
./collector -u http://localhost:8000 -i 5
```

This is useful for:
- Remote monitoring
- Separate collector processes
- Distributed systems

## API Integration

The collector sends metrics to `POST /api/v1/metrics/collect` endpoint.

The JSON format matches the backend API schema:

```json
{
  "cpu_percent": 45.5,
  "cpu_count": 4,
  "cpu_freq_current": 2400.0,
  "memory_total": 8589934592,
  "memory_percent": 50.0,
  "disk_percent": 50.0,
  ...
}
```

Both collectors (psutil and C collector) work with the same API endpoints:

- `POST /api/v1/metrics/collect` - Collect and save metrics
- `GET /api/v1/metrics/live` - Get latest metrics from cache
- `GET /api/v1/metrics/history` - Get historical metrics

The collector type is transparent to the API - same JSON format, same endpoints.

## Running as a Service

### systemd (Linux)

Create `/etc/systemd/system/metrics-collector.service`:

```ini
[Unit]
Description=System Metrics Collector
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/collector -u http://localhost:8000 -i 5
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable metrics-collector
sudo systemctl start metrics-collector
```

### Windows Service

Use NSSM (Non-Sucking Service Manager) or similar tools to run as a Windows service.

## Cleanup

```bash
make clean
```

Removes compiled objects and executable.
