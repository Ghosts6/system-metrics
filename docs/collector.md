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

### Collected Metrics

The collector gathers the following system metrics:

*   **CPU:**
    *   CPU Usage Percentage
    *   Number of CPU Cores
    *   Current, Min, and Max CPU Frequencies (MHz)
    *   **CPU Brand**
    *   **CPU Vendor ID**
*   **Memory:**
    *   Total, Available, and Used Memory (bytes)
    *   Memory Usage Percentage
*   **Disk:**
    *   Total, Used, and Free Disk Space (bytes)
    *   Disk Usage Percentage
*   **Network:**
    *   Bytes Sent and Received (bytes)
*   **System Information:**
    *   Hostname
    *   Platform (Operating System)
    *   System Uptime (seconds)
*   **GPU (if available):**
    *   GPU Name
    *   Driver Version
    *   Total and Used GPU Memory
    *   Temperature
    *   Utilization


## GPU Metrics Collection

Currently, the C collector supports NVIDIA GPUs on Linux platforms. It leverages the `nvidia-smi` command-line utility to gather detailed GPU metrics including utilization, temperature, memory usage, GPU name, and driver version.

### Host Prerequisites for GPU Collection

For the collector container to access host GPU resources and `nvidia-smi`, the host system must have the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) installed and configured for Docker. This enables GPU passthrough to Docker containers.

### Future Enhancements for GPU Support

Future plans include expanding GPU metrics collection to:

-   **Other Linux GPUs**: Implement support for AMD (`rocm-smi`) and Intel GPUs.
-   **Windows**: Integrate with Windows-specific APIs or vendor-provided tools for NVIDIA, AMD, and Intel GPUs.
-   **macOS**: Utilize macOS-specific frameworks for GPU monitoring.

This will provide a comprehensive, cross-platform GPU monitoring solution.

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
  "network_bytes_sent": 1024000,
  "network_bytes_recv": 2048000,
  "hostname": "example-host",
  "platform": "Linux",
  "uptime_seconds": 123456.78,
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
