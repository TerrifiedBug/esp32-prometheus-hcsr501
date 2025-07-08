# ESP32 Prometheus HC-SR501 PIR Motion Sensor

A simple Arduino sketch for ESP32 that monitors motion using an HC-SR501 PIR (Passive Infrared) sensor and exposes metrics in Prometheus format over HTTP.

## Features

- **Motion Detection**: Real-time motion detection using HC-SR501 PIR sensor
- **Prometheus Metrics**: Exposes metrics in Prometheus format at `/metrics` endpoint
- **Web Interface**: Simple web dashboard showing current status
- **Health Check**: JSON health endpoint at `/health`
- **WiFi Connectivity**: Connects to WiFi network with fallback handling
- **System Monitoring**: Tracks uptime, memory usage, and WiFi signal strength

## Hardware Requirements

- ESP32 development board
- HC-SR501 PIR Motion Detection Module
- Jumper wires
- Breadboard (optional)

## Wiring Diagram

```
HC-SR501 PIR Sensor    ESP32
-------------------    -----
VCC                 -> 3.3V or 5V
GND                 -> GND
OUT                 -> GPIO 2 (configurable)
```

## Prometheus Metrics

The following metrics are exposed at the `/metrics` endpoint:

| Metric Name                    | Type    | Description                                                     |
| ------------------------------ | ------- | --------------------------------------------------------------- |
| `pir_motion_detected`          | gauge   | Current motion detection state (0=no motion, 1=motion detected) |
| `pir_motion_count_total`       | counter | Total number of motion detections since boot                    |
| `pir_last_motion_time_seconds` | gauge   | Timestamp of last motion detection                              |
| `esp32_uptime_seconds`         | counter | System uptime in seconds                                        |
| `wifi_signal_strength_dbm`     | gauge   | WiFi signal strength in dBm                                     |
| `esp32_free_heap_bytes`        | gauge   | Free heap memory in bytes                                       |

## Installation

1. **Install Arduino IDE** or **PlatformIO**
2. **Install ESP32 Board Package**:

   - In Arduino IDE: File → Preferences → Additional Board Manager URLs
   - Add: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Go to Tools → Board → Boards Manager, search for "ESP32" and install

3. **Clone this repository**:

   ```bash
   git clone https://github.com/yourusername/esp32-prometheus-hcsr501.git
   cd esp32-prometheus-hcsr501
   ```

4. **Configure WiFi credentials**:

   - Open `src/esp32_pir_sensor.ino`
   - Update `ssid` and `password` variables with your WiFi credentials

5. **Upload to ESP32**:
   - Connect your ESP32 to your computer
   - Select the correct board and port in Arduino IDE
   - Upload the sketch

## Configuration

### WiFi Settings

Edit the following lines in `src/esp32_pir_sensor.ino`:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### PIR Sensor Pin

The PIR sensor is connected to GPIO 2 by default. To change this:

```cpp
const int pirPin = 2; // Change to your desired GPIO pin
```

## Usage

1. **Power on** your ESP32 with the PIR sensor connected
2. **Monitor Serial Output** (115200 baud) to see the IP address
3. **Access Web Interface**: Navigate to `http://<ESP32_IP>/` in your browser
4. **View Metrics**: Access Prometheus metrics at `http://<ESP32_IP>/metrics`
5. **Health Check**: Check system health at `http://<ESP32_IP>/health`

## Prometheus Configuration

Add the following to your `prometheus.yml` configuration:

```yaml
scrape_configs:
  - job_name: "esp32-pir-sensor"
    static_configs:
      - targets: ["<ESP32_IP>:80"]
    scrape_interval: 15s
    metrics_path: /metrics
```

## API Endpoints

- `GET /` - Web dashboard with current status
- `GET /metrics` - Prometheus metrics endpoint
- `GET /health` - JSON health check endpoint

## Troubleshooting

### WiFi Connection Issues

- Verify SSID and password are correct
- Check if your network supports 2.4GHz (ESP32 doesn't support 5GHz)
- Ensure the ESP32 is within range of your WiFi router

### PIR Sensor Not Working

- Check wiring connections
- Verify power supply (3.3V or 5V depending on your PIR module)
- Allow 1-2 minutes for PIR sensor calibration after power-on
- Adjust sensitivity potentiometer on the PIR module if needed

### Serial Monitor Shows No Output

- Check baud rate is set to 115200
- Verify correct COM port is selected
- Try pressing the reset button on ESP32

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- ESP32 Arduino Core developers
- Prometheus community for metrics standards
- HC-SR501 PIR sensor documentation and community examples
