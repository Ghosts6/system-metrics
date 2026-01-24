# system-metrics

![AIAgent](/frontend/public/img/Baner.png?raw=true)

Cross-platform system metrics and log analytics platform.

## Architecture

- Qt (C++) desktop client
- FastAPI backend service
- Redis for live-state caching
- SQLite/PostgreSQL for persistence
- Low-level C collectors

## Quick Start

### 1. Start Backend Services

```bash
sudo docker-compose up -d
```

This starts:
- Backend API on `http://localhost:8000`
- PostgreSQL database
- Redis cache

Verify backend is running:
```bash
curl http://localhost:8000/api/v1/health
```

### 2. Build Qt Client

**Install Dependencies:**
```bash
# Ubuntu/Debian
sudo apt-get install qt6-base-dev qt6-charts-dev qt6-tools-dev cmake build-essential
```

**Build:**
```bash
cd client
mkdir -p build && cd build
cmake ..
make
```

### 3. Run Qt Client

```bash
./system-metrics-client
```

**In the client:**
1. Default URL is `http://localhost:8000`
2. Click **"Connect"** to fetch data
3. Click **"Start Auto-Refresh"** for live updates (5s interval)
4. Switch between **"Metrics"** and **"Logs"** tabs

## Features

- **Real-time Metrics**: CPU, memory, disk, network monitoring
- **Log Viewer**: Filterable system logs with pagination
- **Modern UI**: Dark theme with green accents and smooth animations
- **Auto-refresh**: Configurable live data updates
- **Cross-platform**: Linux, macOS, Windows support

## Documentation

- [API Documentation](docs/api.md) - REST API endpoints
- [Setup Guide](docs/setup.md) - Docker setup and testing
- [C Collector](docs/collector.md) - C metrics collector and backend integration
- [Qt Client](docs/client.md) - Client dependencies and usage

## Project Structure

```
system-metrics/
├── backend/          # FastAPI backend service
├── client/           # Qt (C++) desktop client
├── collector-c/       # C metrics collector
├── docs/             # Documentation
└── docker-compose.yml # Docker orchestration
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a pull request or open an issue.

## 📄 License

This project is licensed under the MIT License.

## 🎥 Demo