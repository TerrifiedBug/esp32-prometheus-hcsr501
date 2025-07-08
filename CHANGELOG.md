# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-01-08

### Added

- Enhanced web dashboard with modern responsive design
- Auto-refresh functionality for real-time monitoring
- Additional Prometheus metrics with device labels
- System information endpoint (`/info`)
- Device reset endpoint (`/reset`)
- WiFi reconnection handling with statistics
- Sensor debouncing for more reliable motion detection
- LED status indicator (built-in LED shows WiFi connection status)
- Heartbeat logging for system monitoring
- Request statistics tracking
- JSON-formatted error responses
- Comprehensive system information in health endpoint

### Improved

- Better code organization with separate functions
- Enhanced error handling and logging
- More detailed serial output with structured logging
- Improved WiFi connection timeout handling
- Better memory management
- More robust sensor reading with debouncing

### Changed

- Upgraded web interface with modern CSS styling
- Enhanced Prometheus metrics with device metadata
- Improved health check endpoint with more system information
- Better structured configuration constants

## [1.0.0] - 2025-01-08

### Added

- Initial release
- Basic PIR motion detection
- WiFi connectivity
- Simple web interface
- Prometheus metrics endpoint (`/metrics`)
- Health check endpoint (`/health`)
- Basic motion counting and timing
- Serial logging

### Features

- HC-SR501 PIR sensor integration
- ESP32 WiFi connectivity
- HTTP web server
- Real-time motion detection
- Prometheus-compatible metrics
- System uptime tracking
- Memory usage monitoring
- WiFi signal strength reporting

### Metrics

- `pir_motion_detected` - Current motion state
- `pir_motion_count_total` - Total motion detections
- `pir_last_motion_time_seconds` - Last motion timestamp
- `esp32_uptime_seconds` - System uptime
- `wifi_signal_strength_dbm` - WiFi signal strength
- `esp32_free_heap_bytes` - Available memory
