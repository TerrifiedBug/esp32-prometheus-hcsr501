/*
 * ESP32 PIR Motion Sensor with Prometheus Metrics
 *
 * This sketch monitors motion using an HC-SR501 PIR sensor and exposes
 * metrics in Prometheus format over HTTP.
 *
 * Hardware:
 * - ESP32 development board
 * - HC-SR501 PIR Motion Detection Module
 *
 * Wiring:
 * - PIR VCC -> ESP32 3.3V or 5V
 * - PIR GND -> ESP32 GND
 * - PIR OUT -> ESP32 GPIO 2 (configurable)
 *
 * Author: TerrifiedBug
 * License: MIT
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Configuration constants
const char* FIRMWARE_VERSION = "1.1.0";
const char* DEVICE_NAME = "ESP32-PIR-Sensor";

// WiFi credentials!
const char* ssid = "ssid";
const char* password = "password";

// Hardware configuration
const int PIR_PIN = 2;
const int LED_PIN = 4;

// Network configuration
const int HTTP_PORT = 80;
const int WIFI_TIMEOUT_MS = 30000;
const int WIFI_RETRY_DELAY_MS = 1000;

// Sensor configuration
const int SENSOR_READ_DELAY_MS = 50;
const int MOTION_DEBOUNCE_MS = 100;

// Web server
WebServer server(HTTP_PORT);

// Motion detection variables
volatile bool motionDetected = false;
volatile unsigned long lastMotionTime = 0;
volatile unsigned long motionCount = 0;
int currentMotionState = LOW;
int previousMotionState = LOW;
unsigned long lastDebounceTime = 0;

// System variables
unsigned long bootTime = 0;
unsigned long lastHeartbeat = 0;
const unsigned long HEARTBEAT_INTERVAL = 30000; // 30 seconds

// Statistics
struct SystemStats {
  unsigned long totalRequests = 0;
  unsigned long metricsRequests = 0;
  unsigned long healthRequests = 0;
  unsigned long uptime = 0;
  int wifiReconnects = 0;
};

SystemStats stats;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("ESP32 PIR Motion Sensor");
  Serial.println("Firmware Version: " + String(FIRMWARE_VERSION));
  Serial.println("=================================");

  // Store boot time
  bootTime = millis();

  // Initialize hardware
  initializeHardware();

  // Connect to WiFi
  connectToWiFi();

  // Setup web server
  setupWebServer();

  // Start server
  server.begin();
  Serial.println("HTTP server started on port " + String(HTTP_PORT));

  if (WiFi.status() == WL_CONNECTED) {
    printNetworkInfo();
  }

  Serial.println("System ready!");
  Serial.println("=================================");
}

void loop() {
  // Handle web server requests
  server.handleClient();

  // Read and process PIR sensor
  processPIRSensor();

  // Check WiFi connection and reconnect if needed
  checkWiFiConnection();

  // Periodic heartbeat
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    printHeartbeat();
    lastHeartbeat = millis();
  }

  // Small delay to prevent overwhelming the system
  delay(SENSOR_READ_DELAY_MS);
}

void initializeHardware() {
  Serial.println("Initializing hardware...");

  // Initialize PIR sensor pin
  pinMode(PIR_PIN, INPUT);

  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Hardware initialized");
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi network: " + String(ssid));

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT_MS) {
    delay(WIFI_RETRY_DELAY_MS);
    Serial.print(".");
    attempts++;

    if (attempts % 10 == 0) {
      Serial.println(" (" + String(attempts) + " attempts)");
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("WiFi connected successfully!");
    digitalWrite(LED_PIN, HIGH); // LED on when connected
  } else {
    Serial.println();
    Serial.println("WiFi connection failed! Operating in offline mode.");
    digitalWrite(LED_PIN, LOW);
  }
}

void checkWiFiConnection() {
  static unsigned long lastCheck = 0;
  const unsigned long CHECK_INTERVAL = 10000; // Check every 10 seconds

  if (millis() - lastCheck > CHECK_INTERVAL) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected. Attempting to reconnect...");
      stats.wifiReconnects++;
      digitalWrite(LED_PIN, LOW);
      connectToWiFi();
    }
    lastCheck = millis();
  }
}

void processPIRSensor() {
  int reading = digitalRead(PIR_PIN);

  // Debounce the sensor reading
  if (reading != previousMotionState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > MOTION_DEBOUNCE_MS) {
    if (reading != currentMotionState) {
      currentMotionState = reading;

      if (currentMotionState == HIGH) {
        motionDetected = true;
        lastMotionTime = millis();
        motionCount++;
        Serial.println("Motion detected! Count: " + String(motionCount));
      } else {
        motionDetected = false;
        Serial.println("Motion ended.");
      }
    }
  }

  previousMotionState = reading;
}

void setupWebServer() {
  Serial.println("Setting up web server routes...");

  server.on("/", HTTP_GET, handleRoot);
  server.on("/metrics", HTTP_GET, handleMetrics);
  server.on("/health", HTTP_GET, handleHealth);
  server.on("/info", HTTP_GET, handleInfo);
  server.on("/reset", HTTP_POST, handleReset);
  server.onNotFound(handleNotFound);

  Serial.println("Web server routes configured");
}

void handleRoot() {
  stats.totalRequests++;

  String html = generateWebInterface();
  server.send(200, "text/html", html);
}

String generateWebInterface() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>ESP32 PIR Sensor Dashboard</title>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }";
  html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
  html += ".header { text-align: center; color: #333; border-bottom: 2px solid #007bff; padding-bottom: 10px; }";
  html += ".status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin: 20px 0; }";
  html += ".status-card { background: #f8f9fa; padding: 15px; border-radius: 5px; border-left: 4px solid #007bff; }";
  html += ".status-value { font-size: 24px; font-weight: bold; color: #007bff; }";
  html += ".status-label { color: #666; font-size: 14px; }";
  html += ".motion-active { border-left-color: #28a745 !important; }";
  html += ".motion-active .status-value { color: #28a745; }";
  html += ".links { margin-top: 20px; text-align: center; }";
  html += ".links a { display: inline-block; margin: 5px 10px; padding: 8px 16px; background: #007bff; color: white; text-decoration: none; border-radius: 4px; }";
  html += ".links a:hover { background: #0056b3; }";
  html += ".refresh-btn { background: #28a745; }";
  html += ".refresh-btn:hover { background: #1e7e34; }";
  html += "</style>";
  html += "<script>";
  html += "function refreshPage() { location.reload(); }";
  html += "setInterval(refreshPage, 5000);";
  html += "</script>";
  html += "</head><body>";
  html += "<div class=\"container\">";
  html += "<div class=\"header\">";
  html += "<h1>ESP32 PIR Motion Sensor</h1>";
  html += "<p>Real-time Motion Detection Dashboard</p>";
  html += "</div>";

  html += "<div class=\"status-grid\">";
  html += "<div class=\"status-card " + String(motionDetected ? "motion-active" : "") + "\">";
  html += "<div class=\"status-value\">" + String(motionDetected ? "DETECTED" : "NONE") + "</div>";
  html += "<div class=\"status-label\">Motion Status</div>";
  html += "</div>";

  html += "<div class=\"status-card\">";
  html += "<div class=\"status-value\">" + String(motionCount) + "</div>";
  html += "<div class=\"status-label\">Total Detections</div>";
  html += "</div>";

  html += "<div class=\"status-card\">";
  html += "<div class=\"status-value\">" + String((millis() - bootTime) / 1000) + "s</div>";
  html += "<div class=\"status-label\">Uptime</div>";
  html += "</div>";

  html += "<div class=\"status-card\">";
  html += "<div class=\"status-value\">" + String(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "OFFLINE") + "</div>";
  html += "<div class=\"status-label\">WiFi Status</div>";
  html += "</div>";

  html += "<div class=\"status-card\">";
  html += "<div class=\"status-value\">" + String(ESP.getFreeHeap() / 1024) + " KB</div>";
  html += "<div class=\"status-label\">Free Memory</div>";
  html += "</div>";

  html += "<div class=\"status-card\">";
  html += "<div class=\"status-value\">" + String(stats.totalRequests) + "</div>";
  html += "<div class=\"status-label\">Total Requests</div>";
  html += "</div>";
  html += "</div>";

  html += "<div class=\"links\">";
  html += "<a href=\"/metrics\">Prometheus Metrics</a>";
  html += "<a href=\"/health\">Health Check</a>";
  html += "<a href=\"/info\">System Info</a>";
  html += "<a href=\"javascript:refreshPage()\" class=\"refresh-btn\">Refresh</a>";
  html += "</div>";

  html += "<div style=\"margin-top: 20px; text-align: center; color: #666; font-size: 12px;\">";
  html += "<p>Device: " + String(DEVICE_NAME) + " | Firmware: " + String(FIRMWARE_VERSION) + " | IP: " + WiFi.localIP().toString() + "</p>";
  html += "</div>";
  html += "</div></body></html>";

  return html;
}

void handleMetrics() {
  stats.totalRequests++;
  stats.metricsRequests++;

  String metrics = generatePrometheusMetrics();
  server.send(200, "text/plain", metrics);
}

String generatePrometheusMetrics() {
  String metrics = "";

  // Add device info as labels
  String deviceLabels = "{device=\"" + String(DEVICE_NAME) + "\",firmware=\"" + String(FIRMWARE_VERSION) + "\",ip=\"" + WiFi.localIP().toString() + "\"}";

  // Motion detection metrics
  metrics += "# HELP pir_motion_detected Current motion detection state (0=no motion, 1=motion detected)\n";
  metrics += "# TYPE pir_motion_detected gauge\n";
  metrics += "pir_motion_detected" + deviceLabels + " " + String(motionDetected ? 1 : 0) + "\n";

  metrics += "# HELP pir_motion_count_total Total number of motion detections since boot\n";
  metrics += "# TYPE pir_motion_count_total counter\n";
  metrics += "pir_motion_count_total" + deviceLabels + " " + String(motionCount) + "\n";

  metrics += "# HELP pir_last_motion_time_seconds Timestamp of last motion detection\n";
  metrics += "# TYPE pir_last_motion_time_seconds gauge\n";
  metrics += "pir_last_motion_time_seconds" + deviceLabels + " " + String(lastMotionTime / 1000.0, 3) + "\n";

  // System metrics
  metrics += "# HELP esp32_uptime_seconds System uptime in seconds\n";
  metrics += "# TYPE esp32_uptime_seconds counter\n";
  metrics += "esp32_uptime_seconds" + deviceLabels + " " + String((millis() - bootTime) / 1000.0, 3) + "\n";

  metrics += "# HELP esp32_free_heap_bytes Free heap memory in bytes\n";
  metrics += "# TYPE esp32_free_heap_bytes gauge\n";
  metrics += "esp32_free_heap_bytes" + deviceLabels + " " + String(ESP.getFreeHeap()) + "\n";

  metrics += "# HELP esp32_total_heap_bytes Total heap memory in bytes\n";
  metrics += "# TYPE esp32_total_heap_bytes gauge\n";
  metrics += "esp32_total_heap_bytes" + deviceLabels + " " + String(ESP.getHeapSize()) + "\n";

  // WiFi metrics
  if (WiFi.status() == WL_CONNECTED) {
    metrics += "# HELP wifi_signal_strength_dbm WiFi signal strength in dBm\n";
    metrics += "# TYPE wifi_signal_strength_dbm gauge\n";
    metrics += "wifi_signal_strength_dbm" + deviceLabels + " " + String(WiFi.RSSI()) + "\n";

    metrics += "# HELP wifi_connected WiFi connection status (1=connected, 0=disconnected)\n";
    metrics += "# TYPE wifi_connected gauge\n";
    metrics += "wifi_connected" + deviceLabels + " 1\n";
  } else {
    metrics += "# HELP wifi_connected WiFi connection status (1=connected, 0=disconnected)\n";
    metrics += "# TYPE wifi_connected gauge\n";
    metrics += "wifi_connected" + deviceLabels + " 0\n";
  }

  // HTTP metrics
  metrics += "# HELP http_requests_total Total HTTP requests received\n";
  metrics += "# TYPE http_requests_total counter\n";
  metrics += "http_requests_total" + deviceLabels + " " + String(stats.totalRequests) + "\n";

  metrics += "# HELP http_metrics_requests_total Total metrics endpoint requests\n";
  metrics += "# TYPE http_metrics_requests_total counter\n";
  metrics += "http_metrics_requests_total" + deviceLabels + " " + String(stats.metricsRequests) + "\n";

  return metrics;
}

void handleHealth() {
  stats.totalRequests++;
  stats.healthRequests++;

  DynamicJsonDocument doc(1024);

  doc["status"] = "healthy";
  doc["device"] = DEVICE_NAME;
  doc["firmware"] = FIRMWARE_VERSION;
  doc["uptime_seconds"] = (millis() - bootTime) / 1000.0;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["wifi_reconnects"] = stats.wifiReconnects;
  doc["motion_state"] = motionDetected;
  doc["motion_count"] = motionCount;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["total_heap"] = ESP.getHeapSize();
  doc["ip_address"] = WiFi.localIP().toString();

  if (WiFi.status() == WL_CONNECTED) {
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["wifi_ssid"] = WiFi.SSID();
  }

  String response;
  serializeJsonPretty(doc, response);

  server.send(200, "application/json", response);
}

void handleInfo() {
  stats.totalRequests++;

  DynamicJsonDocument doc(2048);

  doc["device_info"]["name"] = DEVICE_NAME;
  doc["device_info"]["firmware"] = FIRMWARE_VERSION;
  doc["device_info"]["chip_model"] = ESP.getChipModel();
  doc["device_info"]["chip_revision"] = ESP.getChipRevision();
  doc["device_info"]["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  doc["device_info"]["flash_size"] = ESP.getFlashChipSize();

  doc["network"]["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  if (WiFi.status() == WL_CONNECTED) {
    doc["network"]["ip_address"] = WiFi.localIP().toString();
    doc["network"]["mac_address"] = WiFi.macAddress();
    doc["network"]["ssid"] = WiFi.SSID();
    doc["network"]["rssi"] = WiFi.RSSI();
  }

  doc["sensor"]["pir_pin"] = PIR_PIN;
  doc["sensor"]["motion_detected"] = motionDetected;
  doc["sensor"]["motion_count"] = motionCount;
  doc["sensor"]["last_motion_time"] = lastMotionTime;

  doc["statistics"]["total_requests"] = stats.totalRequests;
  doc["statistics"]["metrics_requests"] = stats.metricsRequests;
  doc["statistics"]["health_requests"] = stats.healthRequests;
  doc["statistics"]["wifi_reconnects"] = stats.wifiReconnects;
  doc["statistics"]["uptime_seconds"] = (millis() - bootTime) / 1000.0;

  String response;
  serializeJsonPretty(doc, response);

  server.send(200, "application/json", response);
}

void handleReset() {
  stats.totalRequests++;

  server.send(200, "text/plain", "Resetting device...");
  delay(1000);
  ESP.restart();
}

void handleNotFound() {
  stats.totalRequests++;

  DynamicJsonDocument doc(512);
  doc["error"] = "Not Found";
  doc["uri"] = server.uri();
  doc["method"] = (server.method() == HTTP_GET) ? "GET" : "POST";
  doc["message"] = "The requested resource was not found on this server";

  String response;
  serializeJson(doc, response);

  server.send(404, "application/json", response);
}

void printNetworkInfo() {
  Serial.println("Network Information:");
  Serial.println("  IP Address: " + WiFi.localIP().toString());
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.println("  SSID: " + WiFi.SSID());
  Serial.println("  Signal Strength: " + String(WiFi.RSSI()) + " dBm");
  Serial.println("  Gateway: " + WiFi.gatewayIP().toString());
  Serial.println("  DNS: " + WiFi.dnsIP().toString());
  Serial.println();
  Serial.println("Available endpoints:");
  Serial.println("  Dashboard: http://" + WiFi.localIP().toString() + "/");
  Serial.println("  Metrics: http://" + WiFi.localIP().toString() + "/metrics");
  Serial.println("  Health: http://" + WiFi.localIP().toString() + "/health");
  Serial.println("  Info: http://" + WiFi.localIP().toString() + "/info");
}

void printHeartbeat() {
  Serial.println("Heartbeat - Uptime: " + String((millis() - bootTime) / 1000) + "s, " +
                 "Motion Count: " + String(motionCount) + ", " +
                 "Free Heap: " + String(ESP.getFreeHeap()) + " bytes, " +
                 "WiFi: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
}
