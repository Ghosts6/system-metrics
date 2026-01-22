# system-metrics

Cross-platform system metrics and log analytics platform.

## Architecture

- Qt (C++) desktop client
- FastAPI backend service
- Redis for live-state caching
- SQLite/PostgreSQL for persistence
- low-level C collectors

## Documentation

- [API Documentation](docs/api.md) - REST API endpoints
- [Setup Guide](docs/setup.md) - Docker setup and testing
- [C Collector](docs/collector.md) - C metrics collector and backend integration

## Status

Backend and C collector are implemented and tested. Qt client is in development.

This repository is treated as a production-grade internal system.

