# Qt Client Documentation

## Overview

The Qt (C++) desktop client provides a modern graphical interface for the System Metrics & Log Analytics Platform. It connects to the FastAPI backend via REST API to display real-time system metrics and logs.

## Architecture

The client follows a clean architecture pattern:

- **ApiClient**: Handles all HTTP communication with the backend
- **MetricsWidget**: Displays real-time system metrics with progress bars
- **LogViewer**: Shows system logs in a filterable table
- **AnimatedButton**: Custom button with hover and press animations
- **Style**: Centralized dark/green theme styling

## Dependencies

### Required Qt6 Modules

- **Qt6::Core**: Core functionality
- **Qt6::Gui**: GUI base classes
- **Qt6::Widgets**: Widget-based UI components
- **Qt6::Network**: HTTP client functionality
- **Qt6::Charts**: (Optional) For future chart visualizations
- **Qt6::Concurrent**: (Optional) For background operations

### System Requirements

- **CMake**: 3.16 or higher
- **C++ Compiler**: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)
- **Qt6**: 6.0 or higher
- **Operating System**: Linux, macOS, or Windows

### Installation

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install qt6-base-dev qt6-charts-dev qt6-tools-dev cmake build-essential
```

**macOS:**
```bash
brew install qt6 cmake
```

**Windows:**
- Download Qt6 from https://www.qt.io/download
- Install CMake from https://cmake.org/download/
- Use Visual Studio or MinGW-w64

## Building

```bash
cd client
mkdir -p build
cd build
cmake ..
make
```

The executable will be created as `system-metrics-client` (or `system-metrics-client.exe` on Windows).

## Features

### UI/UX Design

- **Dark Theme**: Modern dark color scheme (#1a1a1a background)
- **Green Accents**: Teal/green color scheme (#0d7377, #14a085)
- **Animated Buttons**: Smooth hover and press animations
- **Progress Bars**: Gradient-filled progress indicators
- **Responsive Layout**: Clean, organized interface

### Functionality

- **Real-time Metrics**: Live system metrics with auto-refresh
- **Log Viewer**: Filterable log table with pagination
- **Connection Management**: Configurable API URL
- **Error Handling**: User-friendly error messages
- **Status Bar**: Connection status and health information

## API Integration

The client communicates with the backend using these endpoints:

- `GET /api/v1/metrics/live` - Fetch latest metrics
- `GET /api/v1/metrics/history` - Get historical metrics
- `GET /api/v1/logs/` - Retrieve logs with filtering
- `POST /api/v1/logs/` - Create log entries
- `GET /api/v1/health` - Check backend health

## Configuration

The client connects to `http://localhost:8000` by default. Change the API URL in the connection panel to point to a different backend instance.

## Usage

1. Start the backend (see main README)
2. Launch the Qt client
3. Click "Connect" to establish connection
4. Click "Start Auto-Refresh" for live updates
5. Switch between "Metrics" and "Logs" tabs

## Code Structure

```
client/
├── src/
│   ├── main.cpp           # Application entry point
│   ├── apiclient.h/cpp     # REST API client
│   ├── metricswidget.h/cpp # Metrics display widget
│   ├── logviewer.h/cpp     # Log viewer widget
│   ├── animatedbutton.h/cpp # Animated button component
│   └── style.h/cpp         # Theme styling
├── CMakeLists.txt          # Build configuration
└── build/                  # Build output directory
```

## Troubleshooting

**Build Errors:**
- Ensure Qt6 is properly installed: `qmake6 --version`
- Check CMake can find Qt6: `cmake -DCMAKE_PREFIX_PATH=/path/to/qt6 ..`

**Connection Issues:**
- Verify backend is running: `curl http://localhost:8000/api/v1/health`
- Check firewall settings
- Ensure correct API URL format (include http://)

**Runtime Errors:**
- Check Qt6 libraries are in library path
- Verify all required Qt modules are installed
