# Qt Client Documentation

## Overview

The System Metrics Client is a cross-platform Qt6 (C++) desktop application for monitoring system metrics and viewing logs. It provides a modern, dark-themed interface with real-time updates and historical data visualization.

## Features

### 1. Dashboard Tab
- **Overview**: Provides a comprehensive overview of system metrics at a glance
- **Key Metrics Display**: Large, easy-to-read displays for CPU, Memory, and Disk usage
- **Visual Indicators**: Progress bars with color-coded status
- **System Information**: Hostname, platform, and connection status
- **Network Activity**: Real-time network statistics

### 2. Live Metrics Tab
- **Real-time System Metrics**: CPU, Memory, Disk, and Network usage
- **Detailed Information**: Shows both percentages and absolute values
- **Progress Bars**: Visual representation of resource usage
- **Live Usage Graphs**: Real-time line charts showing usage trends for each component
  - CPU Usage % graph
  - Memory Usage % graph
  - Disk Usage % graph
  - Network Activity graph (sent/received in MB)
- **Chart Selection**: Dropdown to select which metric to view in detail
- **Rolling History**: Charts maintain last 60 data points (approximately 5 minutes at 5s intervals)
- **Auto-refresh**: Configurable refresh intervals

### 3. Historical Charts Tab
- **Time-series Visualization**: Interactive charts using Qt Charts
- **Multiple Metric Types**: 
  - CPU Usage %
  - Memory Usage %
  - Disk Usage %
  - Network Sent (MB)
  - Network Received (MB)
- **Time Range Selection**:
  - Last Hour
  - Last 6 Hours
  - Last 24 Hours
  - Last 7 Days
- **Live Updates**: Automatic refresh with start/stop controls
- **Auto-scaling**: Charts automatically adjust to data range
- **Default View**: Automatically loads "Last 24 Hours" on startup

### 4. Logs Tab
- **Log Viewer**: Table-based log display with filtering
- **Filtering Options**:
  - By log level (INFO, WARNING, ERROR, DEBUG)
  - By time range (All Time, Last Hour, 6 Hours, 24 Hours, 7 Days)
  - By search term (searches in message and source fields)
- **Pagination**: Navigate through large log sets
- **Log Details**: Double-click any log entry to view full details
- **Export Functionality**: Export logs to CSV or JSON format
- **Create Test Log**: Button to create test log entries for testing
- **Color Coding**: Log levels are color-coded for easy identification
- **Default View**: Shows "All Time" by default

### 5. System Info Tab
- **Comprehensive System Information**: Detailed breakdown of all system metrics
- **Organized Sections**:
  - System Information (hostname, platform, timestamp)
  - CPU Information (cores, usage, frequencies)
  - Memory Information (total, used, available, percentage)
  - Disk Information (total, used, free, percentage)
  - Network Information (bytes sent/received)
- **Formatted Display**: Human-readable byte formatting (KB, MB, GB, TB)

### 6. Settings Dialog
- **API Configuration**: Set API base URL
- **Refresh Interval**: Configure auto-refresh interval (1-3600 seconds)
- **Auto-connect**: Enable/disable automatic connection on startup
- **Notifications**: Toggle notification display
- **Persistent Storage**: Settings are saved and restored on application restart
- **Reset to Defaults**: Option to restore default settings

## Architecture

### Components

1. **ApiClient** (`apiclient.h/cpp`)
   - Handles all HTTP communication with the backend
   - Manages network requests and responses
   - Provides signals for data updates
   - Supports automatic refresh with configurable intervals

2. **MetricsWidget** (`metricswidget.h/cpp`)
   - Displays live system metrics
   - Shows CPU, Memory, Disk, and Network information
   - Real-time usage graphs for each component
   - Chart selection dropdown for detailed view
   - Updates in real-time

3. **MetricsChartWidget** (`metricschartwidget.h/cpp`)
   - Historical metrics visualization using Qt Charts
   - Time-series line charts
   - Multiple metric type support
   - Time range filtering (preset ranges)
   - Live update controls (start/stop)

4. **DashboardWidget** (`dashboardwidget.h/cpp`)
   - Overview dashboard with key metrics
   - Large, readable displays
   - Status indicators

5. **LogViewer** (`logviewer.h/cpp`)
   - Log viewing and filtering
   - Search functionality
   - Export capabilities
   - Log detail dialog

6. **SystemInfoWidget** (`systeminfowidget.h/cpp`)
   - Detailed system information display
   - Organized metric sections
   - Formatted data presentation

7. **SettingsDialog** (`settingsdialog.h/cpp`)
   - Application configuration
   - Persistent settings storage
   - User preferences management

8. **AnimatedButton** (`animatedbutton.h/cpp`)
   - Custom button with hover animations
   - Enhanced user experience

9. **Style** (`style.h/cpp`)
   - Centralized styling
   - Dark green theme
   - Consistent UI appearance

## Building

### Requirements
- Qt6 (Core, Gui, Widgets, Network, Charts, Concurrent)
- CMake 3.16 or higher
- C++17 compiler

### Build Steps

```bash
cd client
mkdir -p build
cd build
cmake ..
make
```

The executable will be created as `system-metrics-client` in the build directory.

## Usage

### Connection
1. Enter the API URL in the connection bar (default: `http://localhost:8000`)
2. Click "Connect" or the connection will be established automatically if auto-connect is enabled
3. Connection status is indicated by the colored dot (green = connected, red = disconnected)

### Live Updates
- Click "Start Live Updates" to begin automatic refresh
- Refresh interval can be configured in Settings
- Click "Stop Live Updates" to pause automatic refresh

### Viewing Historical Data
1. Navigate to the "Historical Charts" tab
2. Select a metric type from the dropdown
3. Choose a time range from the dropdown (Last Hour, 6 Hours, 24 Hours, 7 Days)
4. Data loads automatically, or click "Refresh" to manually reload
5. Use "Start/Stop Live Update" buttons to control automatic refresh

### Filtering Logs
1. Navigate to the "Logs" tab
2. Use the level dropdown to filter by log level
3. Select a time range from the dropdown (All Time, Last Hour, 6 Hours, 24 Hours, 7 Days)
4. Enter search terms in the search box (searches in message and source)
5. Click "Refresh" to apply filters
6. Use "Create Test Log" to add test entries for testing purposes

### Exporting Logs
1. In the Logs tab, apply any desired filters
2. Click the "Export" button
3. Choose file format (CSV or JSON)
4. Select save location
5. Logs will be exported with current filters applied

### Viewing Log Details
- Double-click any log entry in the table to view full details
- The detail dialog shows all log fields including metadata

### Settings
- Access Settings from the File menu
- Configure API URL, refresh intervals, and preferences
- Settings are automatically saved

### Menu Bar
- **Logo & App Name**: Displayed at the start of the menu bar
- **File Menu**: Settings and Exit options
- **View Menu**: Refresh All option
- **Help Menu**: About dialog with application information and logo

## API Integration

The client communicates with the backend via REST API:

- `GET /api/v1/health` - Health check
- `GET /api/v1/metrics/live` - Get latest metrics
- `GET /api/v1/metrics/history` - Get historical metrics (with time filtering)
- `GET /api/v1/logs/` - Get logs (with level, time, and search filtering)
- `POST /api/v1/logs/` - Create log entry

All API calls are asynchronous and use Qt's signal/slot mechanism for updates.

## Error Handling

- Network errors are displayed in the status bar
- Connection status is visually indicated
- Failed requests are logged but don't crash the application
- Automatic retry on connection loss (when auto-connect is enabled)

## Styling

The application uses a dark green theme with:
- Dark background (#1a1a1a)
- Green accent color (#14a085, #0d7377)
- Consistent styling across all widgets
- Responsive UI elements with hover effects
- Professional dropdown menus with custom styling
- Application logo displayed in menu bar and About dialog

## Best Practices

1. **Separation of Concerns**: UI components are separate from API logic
2. **Signal/Slot Architecture**: Loose coupling between components
3. **Resource Management**: Proper memory management with Qt parent-child relationships
4. **Error Handling**: Graceful degradation on errors
5. **User Experience**: Responsive UI with visual feedback
6. **Code Standards**: Follows Qt coding conventions and C++17 standards

## Future Enhancements

Potential improvements:
- WebSocket support for real-time push updates
- Export metrics to various formats
- Alert/notification system for threshold breaches
- Multiple server connections
- Customizable dashboard layouts
- Metric aggregation and statistics
- Process monitoring
- Network interface details
