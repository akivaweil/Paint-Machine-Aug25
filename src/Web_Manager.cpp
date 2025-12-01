#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "Web_Manager.h"
#include "config/Config.h"
#include "config/Pin_Definitions.h"

//* ************************************************************************
//* ************************ WEB MANAGER ***********************************
//* ************************************************************************

// Web server instance
AsyncWebServer server(80);
Preferences preferences;

// Move request variables (Direct Move)
volatile bool webMoveRequested = false;
volatile float webTargetX = 0.0;
volatile float webTargetY = 0.0;

// Sequence start request variable
volatile bool webStartRequested = false;

// Test position request variable
volatile bool webTestPositionRequested = false;

// Pick and Place Sequence Variables
volatile float webPickX = 0.0;
volatile float webPickY = 0.0;
volatile float webPlaceX = 0.0;
volatile float webPlaceY = 0.0;
volatile float webPickForkDistance = 0.0;
volatile float webPlaceForkDistance = 0.0;

// Sensor pins initialized flag
bool sensorsInitialized = false;

// HTML Content
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Paint Machine Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --primary-color: #3b82f6;
      --secondary-color: #94a3b8;
      --accent-color: #10b981;
      --bg-color: #0f172a;
      --card-bg: #1e293b;
      --text-color: #f8fafc;
      --input-bg: #334155;
      --input-border: #475569;
      --border-radius: 16px;
      --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.5), 0 2px 4px -1px rgba(0, 0, 0, 0.3);
    }
    
    body {
      font-family: 'Roboto', sans-serif;
      background-color: var(--bg-color);
      color: var(--text-color);
      margin: 0;
      padding: 20px;
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
    }

    .header {
      text-align: center;
      margin-bottom: 40px;
      margin-top: 20px;
    }
    
    .header h1 {
      font-weight: 700;
      color: var(--primary-color);
      margin: 0;
      font-size: 2.2rem;
      text-transform: uppercase;
      letter-spacing: 1.5px;
      text-shadow: 0 2px 4px rgba(0,0,0,0.3);
    }

    .header p {
      color: var(--secondary-color);
      margin-top: 8px;
      font-size: 1.1rem;
    }

    .grid-container {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 25px;
      width: 100%;
      max-width: 1000px;
    }

    .card {
      background: var(--card-bg);
      padding: 30px;
      border-radius: var(--border-radius);
      box-shadow: var(--shadow);
      transition: transform 0.2s ease, box-shadow 0.2s ease;
      display: flex;
      flex-direction: column;
      border: 1px solid rgba(255,255,255,0.05);
    }
    
    .card:hover {
      transform: translateY(-2px);
      box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.5), 0 4px 6px -2px rgba(0, 0, 0, 0.3);
      border-color: rgba(255,255,255,0.1);
    }

    .card h2 {
      margin-top: 0;
      margin-bottom: 25px;
      font-size: 1.25rem;
      color: var(--text-color);
      border-bottom: 2px solid rgba(255,255,255,0.1);
      padding-bottom: 15px;
      display: flex;
      align-items: center;
      text-transform: uppercase;
      letter-spacing: 0.5px;
      font-weight: 600;
    }
    
    .card h2::before {
      content: '';
      display: inline-block;
      width: 6px;
      height: 24px;
      background-color: var(--primary-color);
      margin-right: 12px;
      border-radius: 3px;
      box-shadow: 0 0 10px var(--primary-color);
    }

    .input-group {
      margin-bottom: 20px;
    }

    label {
      display: block;
      margin-bottom: 8px;
      font-weight: 500;
      font-size: 0.9rem;
      color: var(--secondary-color);
    }

    input[type="number"] {
      width: 100%;
      padding: 14px;
      border: 2px solid var(--input-border);
      border-radius: 10px;
      font-size: 1.1rem;
      transition: all 0.3s;
      box-sizing: border-box;
      background: var(--input-bg);
      color: white;
    }

    input[type="number"]:focus {
      border-color: var(--primary-color);
      outline: none;
      background: #405570;
      box-shadow: 0 0 0 2px rgba(59, 130, 246, 0.2);
    }

    .btn {
      width: 100%;
      padding: 16px;
      border: none;
      border-radius: 10px;
      font-size: 1rem;
      font-weight: 700;
      cursor: pointer;
      transition: all 0.2s;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-top: auto;
      color: white;
    }

    .btn:active {
      transform: scale(0.98);
    }

    .btn-primary {
      background: linear-gradient(135deg, #3b82f6 0%, #2563eb 100%);
      box-shadow: 0 4px 6px -1px rgba(37, 99, 235, 0.3);
    }
    
    .btn-primary:hover {
      background: linear-gradient(135deg, #60a5fa 0%, #3b82f6 100%);
      box-shadow: 0 6px 12px rgba(37, 99, 235, 0.4);
    }

    .btn-success {
      background: linear-gradient(135deg, #10b981 0%, #059669 100%);
      box-shadow: 0 4px 6px -1px rgba(16, 185, 129, 0.3);
    }
    
    .btn-success:hover {
      background: linear-gradient(135deg, #34d399 0%, #10b981 100%);
      box-shadow: 0 6px 12px rgba(16, 185, 129, 0.4);
    }

    .btn-start {
      background: linear-gradient(135deg, #f59e0b 0%, #d97706 100%);
      box-shadow: 0 4px 6px -1px rgba(245, 158, 11, 0.3);
      margin-top: 15px;
    }
    
    .btn-start:hover {
      background: linear-gradient(135deg, #fbbf24 0%, #f59e0b 100%);
      box-shadow: 0 6px 12px rgba(245, 158, 11, 0.4);
    }

    .full-width {
      grid-column: 1 / -1;
    }

    .status-bar {
      position: fixed;
      bottom: 30px;
      left: 50%;
      transform: translateX(-50%);
      background-color: rgba(30, 41, 59, 0.95);
      color: white;
      padding: 16px 32px;
      border-radius: 50px;
      box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.5);
      display: none;
      z-index: 1000;
      font-weight: 500;
      letter-spacing: 0.5px;
      min-width: 200px;
      text-align: center;
      border: 1px solid rgba(255,255,255,0.1);
      backdrop-filter: blur(5px);
    }

    .row {
      display: flex;
      gap: 20px;
    }
    
    .col {
      flex: 1;
    }

    /* Placeholder styling */
    ::placeholder {
      color: #64748b;
      opacity: 1;
    }

    @media (max-width: 768px) {
      .grid-container {
        grid-template-columns: 1fr;
      }
      .full-width {
        grid-column: auto;
      }
      .header h1 {
        font-size: 1.8rem;
      }
    }
  </style>
</head>
<body onload="loadConfig()">
  <div class="header">
    <h1>Paint Machine</h1>
    <p>Control Dashboard</p>
  </div>

  <div class="grid-container">
    <!-- Pick Card -->
    <div class="card">
      <h2>Pick Settings</h2>
      <div class="input-group">
        <label for="pickX">Pick X Coordinate (in)</label>
        <input type="number" id="pickX" step="0.1" placeholder="0.0">
      </div>
      <div class="input-group">
        <label for="pickY">Pick Y Coordinate (in)</label>
        <input type="number" id="pickY" step="0.1" placeholder="0.0">
      </div>
      <div class="input-group">
        <label for="pickForkDist">Pick Fork Distance (in)</label>
        <input type="number" id="pickForkDist" step="0.1" placeholder="0.0">
      </div>
    </div>

    <!-- Place Card -->
    <div class="card">
      <h2>Place Settings</h2>
      <div class="input-group">
        <label for="placeX">Place X Coordinate (in)</label>
        <input type="number" id="placeX" step="0.1" placeholder="0.0">
      </div>
      <div class="input-group">
        <label for="placeY">Place Y Coordinate (in)</label>
        <input type="number" id="placeY" step="0.1" placeholder="0.0">
      </div>
      <div class="input-group">
        <label for="placeForkDist">Place Fork Distance (in)</label>
        <input type="number" id="placeForkDist" step="0.1" placeholder="0.0">
      </div>
    </div>

    <!-- Action Card -->
    <div class="card full-width">
      <h2>Actions</h2>
      <button class="btn btn-success" onclick="saveConfig()">Save All Configuration</button>
      <button class="btn btn-start" onclick="startSequence()">START SEQUENCE</button>
      <button class="btn btn-primary" onclick="testPosition()" style="margin-top: 15px;">TEST POSITION</button>
    </div>

    <!-- Direct Move Card -->
    <div class="card full-width">
      <h2 style="border-bottom-color: rgba(255,255,255,0.1);">
        <span style="background-color: var(--primary-color); width: 6px; height: 24px; border-radius: 3px; margin-right: 12px; box-shadow: 0 0 10px var(--primary-color);"></span>
        Manual Control
      </h2>
      <div class="row" style="margin-bottom: 10px;">
        <div class="col">
          <div class="input-group">
            <label for="moveX">Target X</label>
            <input type="number" id="moveX" step="0.1" placeholder="0.0">
          </div>
        </div>
        <div class="col">
          <div class="input-group">
            <label for="moveY">Target Y</label>
            <input type="number" id="moveY" step="0.1" placeholder="0.0">
          </div>
        </div>
      </div>
      <button class="btn btn-primary" onclick="sendMove()">Move Now</button>
    </div>
  </div>

  <div id="status" class="status-bar"></div>

  <script>
    function loadConfig() {
        // Fetch current config from ESP32
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/get_config", true);
        xhr.onreadystatechange = function() {
            if (xhr.readyState == 4 && xhr.status == 200) {
                var data = JSON.parse(xhr.responseText);
                document.getElementById("pickX").value = data.pickX;
                document.getElementById("pickY").value = data.pickY;
                document.getElementById("placeX").value = data.placeX;
                document.getElementById("placeY").value = data.placeY;
                document.getElementById("pickForkDist").value = data.pickForkDist;
                document.getElementById("placeForkDist").value = data.placeForkDist;
            }
        };
        xhr.send();
    }
    
    function showStatus(msg) {
        var statusDiv = document.getElementById("status");
        statusDiv.style.display = "block";
        statusDiv.innerHTML = msg;
        setTimeout(function() { statusDiv.style.display = "none"; }, 3000);
    }

    function sendMove() {
      var x = document.getElementById("moveX").value;
      var y = document.getElementById("moveY").value;
      
      if(x === "" || y === "") {
        showStatus("⚠️ Please enter both X and Y");
        return;
      }
      
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/move?x=" + x + "&y=" + y, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          showStatus("🚀 Move command sent!");
        }
      };
      xhr.send();
    }

    function saveConfig() {
      var px = document.getElementById("pickX").value;
      var py = document.getElementById("pickY").value;
      var plx = document.getElementById("placeX").value;
      var ply = document.getElementById("placeY").value;
      var pfd = document.getElementById("pickForkDist").value;
      var plfd = document.getElementById("placeForkDist").value;

      if(px === "" || py === "" || plx === "" || ply === "" || pfd === "" || plfd === "") {
        showStatus("⚠️ Please fill in all fields");
        return;
      }

      var url = "/config?px=" + px + "&py=" + py + "&plx=" + plx + "&ply=" + ply + "&pfd=" + pfd + "&plfd=" + plfd;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", url, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          showStatus("✅ Configuration Saved!");
        }
      };
      xhr.send();
    }

    function startSequence() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/start_sequence", true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          showStatus("🎬 Sequence Started!");
        }
      };
      xhr.send();
    }

    function testPosition() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/test_position", true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          showStatus("📍 Moving to Test Position!");
        }
      };
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

// Sensor Dashboard HTML
const char sensors_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Sensor Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg-color: #0a0e1a;
      --card-bg: #151b2e;
      --card-border: #1e2a47;
      --text-primary: #e2e8f0;
      --text-secondary: #94a3b8;
      --accent-blue: #3b82f6;
      --accent-green: #10b981;
      --accent-red: #ef4444;
      --accent-yellow: #f59e0b;
      --border-radius: 12px;
      --shadow: 0 8px 16px rgba(0, 0, 0, 0.4);
    }
    
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: 'Roboto', sans-serif;
      background: linear-gradient(135deg, var(--bg-color) 0%, #0f172a 100%);
      color: var(--text-primary);
      min-height: 100vh;
      padding: 20px;
    }
    
    .container {
      max-width: 1200px;
      margin: 0 auto;
    }
    
    .header {
      text-align: center;
      margin-bottom: 40px;
      padding: 30px 0;
    }
    
    .header h1 {
      font-size: 2.5rem;
      font-weight: 700;
      background: linear-gradient(135deg, var(--accent-blue) 0%, var(--accent-green) 100%);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
      margin-bottom: 10px;
      letter-spacing: 2px;
    }
    
    .header p {
      color: var(--text-secondary);
      font-size: 1.1rem;
    }
    
    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    
    .sensor-card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: var(--border-radius);
      padding: 24px;
      box-shadow: var(--shadow);
      transition: all 0.3s ease;
      position: relative;
      overflow: hidden;
    }
    
    .sensor-card::before {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      right: 0;
      height: 3px;
      background: var(--card-border);
      transition: background 0.3s ease;
    }
    
    .sensor-card.active::before {
      background: linear-gradient(90deg, var(--accent-green) 0%, var(--accent-blue) 100%);
      box-shadow: 0 0 20px rgba(59, 130, 246, 0.5);
    }
    
    .sensor-card:hover {
      transform: translateY(-4px);
      box-shadow: 0 12px 24px rgba(0, 0, 0, 0.5);
      border-color: var(--accent-blue);
    }
    
    .sensor-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 16px;
    }
    
    .sensor-name {
      font-size: 1.1rem;
      font-weight: 600;
      color: var(--text-primary);
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    
    .sensor-status {
      display: flex;
      align-items: center;
      gap: 8px;
    }
    
    .status-indicator {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: var(--text-secondary);
      transition: all 0.3s ease;
      box-shadow: 0 0 0 0 rgba(148, 163, 184, 0.4);
    }
    
    .sensor-card.active .status-indicator {
      background: var(--accent-green);
      box-shadow: 0 0 0 4px rgba(16, 185, 129, 0.3), 0 0 20px rgba(16, 185, 129, 0.5);
      animation: pulse 2s infinite;
    }
    
    @keyframes pulse {
      0%, 100% {
        box-shadow: 0 0 0 4px rgba(16, 185, 129, 0.3), 0 0 20px rgba(16, 185, 129, 0.5);
      }
      50% {
        box-shadow: 0 0 0 8px rgba(16, 185, 129, 0.1), 0 0 30px rgba(16, 185, 129, 0.7);
      }
    }
    
    .status-text {
      font-size: 0.85rem;
      font-weight: 500;
      color: var(--text-secondary);
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    
    .sensor-card.active .status-text {
      color: var(--accent-green);
    }
    
    .sensor-info {
      margin-top: 12px;
      padding-top: 12px;
      border-top: 1px solid var(--card-border);
    }
    
    .sensor-info-item {
      display: flex;
      justify-content: space-between;
      margin-bottom: 8px;
      font-size: 0.9rem;
    }
    
    .sensor-info-label {
      color: var(--text-secondary);
    }
    
    .sensor-info-value {
      color: var(--text-primary);
      font-weight: 500;
    }
    
    .footer {
      text-align: center;
      margin-top: 40px;
      padding: 20px;
      color: var(--text-secondary);
      font-size: 0.9rem;
    }
    
    .last-update {
      color: var(--text-secondary);
      font-size: 0.85rem;
      margin-top: 20px;
      text-align: center;
    }
    
    @media (max-width: 768px) {
      .sensor-grid {
        grid-template-columns: 1fr;
      }
      .header h1 {
        font-size: 2rem;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Sensor Dashboard</h1>
      <p>Real-time Sensor Status Monitor</p>
    </div>
    
    <div class="sensor-grid" id="sensorGrid">
      <!-- Sensors will be populated by JavaScript -->
    </div>
    
    <div class="last-update" id="lastUpdate">Last update: --</div>
    
    <div class="footer">
      Paint Machine Control System
    </div>
  </div>
  
  <script>
    const sensors = [
      { id: 'xHome1', name: 'X Home Switch 1', pin: 'Pin 7' },
      { id: 'xHome2', name: 'X Home Switch 2', pin: 'Pin 8' },
      { id: 'yHome', name: 'Y Home Switch', pin: 'Pin 4' },
      { id: 'forkHome', name: 'Fork Home Switch', pin: 'Pin 18' },
      { id: 'testButton', name: 'Test Button', pin: 'Pin 38' }
    ];
    
    function createSensorCard(sensor, state) {
      const isActive = state === true || state === 1;
      return `
        <div class="sensor-card ${isActive ? 'active' : ''}" id="card-${sensor.id}">
          <div class="sensor-header">
            <div class="sensor-name">${sensor.name}</div>
            <div class="sensor-status">
              <div class="status-indicator"></div>
              <span class="status-text">${isActive ? 'TRIGGERED' : 'IDLE'}</span>
            </div>
          </div>
          <div class="sensor-info">
            <div class="sensor-info-item">
              <span class="sensor-info-label">Pin:</span>
              <span class="sensor-info-value">${sensor.pin}</span>
            </div>
            <div class="sensor-info-item">
              <span class="sensor-info-label">State:</span>
              <span class="sensor-info-value">${isActive ? 'HIGH' : 'LOW'}</span>
            </div>
          </div>
        </div>
      `;
    }
    
    function updateSensors() {
      fetch('/api/sensors')
        .then(response => response.json())
        .then(data => {
          const grid = document.getElementById('sensorGrid');
          grid.innerHTML = sensors.map(sensor => {
            const state = data[sensor.id];
            return createSensorCard(sensor, state);
          }).join('');
          
          const now = new Date();
          document.getElementById('lastUpdate').textContent = 
            `Last update: ${now.toLocaleTimeString()}`;
        })
        .catch(error => {
          console.error('Error fetching sensor data:', error);
        });
    }
    
    // Update immediately and then every 200ms
    updateSensors();
    setInterval(updateSensors, 200);
  </script>
</body>
</html>
)rawliteral";

// Initialize sensor pins
void initializeSensors() {
    if (sensorsInitialized) return;
    
    // Initialize home switches (Active HIGH with pulldown)
    pinMode(X_HOME_PIN, INPUT_PULLDOWN);
    pinMode(X_HOME_PIN2, INPUT_PULLDOWN);
    pinMode(Y_HOME_PIN, INPUT_PULLDOWN);
    pinMode(FORK_HOME_PIN, INPUT_PULLDOWN);
    
    // Initialize test button (assuming active LOW with pullup, adjust if needed)
    pinMode(TEST_BUTTON_PIN, INPUT_PULLUP);
    
    sensorsInitialized = true;
}

// Read sensor states
String getSensorStatesJSON() {
    bool xHome1 = digitalRead(X_HOME_PIN);
    bool xHome2 = digitalRead(X_HOME_PIN2);
    bool yHome = digitalRead(Y_HOME_PIN);
    bool forkHome = digitalRead(FORK_HOME_PIN);
    bool testButton = !digitalRead(TEST_BUTTON_PIN); // Inverted for pullup
    
    String json = "{";
    json += "\"xHome1\":" + String(xHome1 ? "true" : "false") + ",";
    json += "\"xHome2\":" + String(xHome2 ? "true" : "false") + ",";
    json += "\"yHome\":" + String(yHome ? "true" : "false") + ",";
    json += "\"forkHome\":" + String(forkHome ? "true" : "false") + ",";
    json += "\"testButton\":" + String(testButton ? "true" : "false");
    json += "}";
    
    return json;
}

void initWebServer() {
    // Initialize sensor pins
    initializeSensors();
    
    // Initialize Preferences
    preferences.begin("paint-config", false);
    
    // Load saved values
    webPickX = preferences.getFloat("pickX", 0.0);
    webPickY = preferences.getFloat("pickY", 0.0);
    webPlaceX = preferences.getFloat("placeX", 0.0);
    webPlaceY = preferences.getFloat("placeY", 0.0);
    webPickForkDistance = preferences.getFloat("pickForkDist", 0.0);
    webPlaceForkDistance = preferences.getFloat("placeForkDist", 0.0);
    
    Serial.println("Loaded Config from NVS:");
    Serial.printf("Pick: %.2f, %.2f (Fork: %.2f)\n", webPickX, webPickY, webPickForkDistance);
    Serial.printf("Place: %.2f, %.2f (Fork: %.2f)\n", webPlaceX, webPlaceY, webPlaceForkDistance);

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });
    
    // Route for sensor dashboard
    server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", sensors_html);
    });
    
    // API endpoint for sensor states
    server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorStatesJSON();
        request->send(200, "application/json", json);
    });

    // Route to get current config as JSON
    server.on("/get_config", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"pickX\":" + String(webPickX) + ",";
        json += "\"pickY\":" + String(webPickY) + ",";
        json += "\"placeX\":" + String(webPlaceX) + ",";
        json += "\"placeY\":" + String(webPlaceY) + ",";
        json += "\"pickForkDist\":" + String(webPickForkDistance) + ",";
        json += "\"placeForkDist\":" + String(webPlaceForkDistance);
        json += "}";
        request->send(200, "application/json", json);
    });

    // Route to handle move command
    server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("x") && request->hasParam("y")) {
            String xVal = request->getParam("x")->value();
            String yVal = request->getParam("y")->value();
            
            webTargetX = xVal.toFloat();
            webTargetY = yVal.toFloat();
            webMoveRequested = true;
            
            request->send(200, "text/plain", "OK");
            Serial.print("Web Request: Move to X=");
            Serial.print(webTargetX);
            Serial.print(", Y=");
            Serial.println(webTargetY);
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });

    // Route to handle start sequence
    server.on("/start_sequence", HTTP_GET, [](AsyncWebServerRequest *request){
        webStartRequested = true;
        request->send(200, "text/plain", "Sequence Started");
        Serial.println("=== WEB SEQUENCE START REQUESTED ===");
    });

    // Route to handle test position request
    server.on("/test_position", HTTP_GET, [](AsyncWebServerRequest *request){
        webTestPositionRequested = true;
        request->send(200, "text/plain", "Test Position Started");
        Serial.println("=== WEB TEST POSITION REQUESTED ===");
    });

    // Route to handle configuration
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("px") && request->hasParam("py") && 
            request->hasParam("plx") && request->hasParam("ply") && 
            request->hasParam("pfd") && request->hasParam("plfd")) {
            
            webPickX = request->getParam("px")->value().toFloat();
            webPickY = request->getParam("py")->value().toFloat();
            webPlaceX = request->getParam("plx")->value().toFloat();
            webPlaceY = request->getParam("ply")->value().toFloat();
            webPickForkDistance = request->getParam("pfd")->value().toFloat();
            webPlaceForkDistance = request->getParam("plfd")->value().toFloat();
            
            // Save to Preferences
            preferences.putFloat("pickX", webPickX);
            preferences.putFloat("pickY", webPickY);
            preferences.putFloat("placeX", webPlaceX);
            preferences.putFloat("placeY", webPlaceY);
            preferences.putFloat("pickForkDist", webPickForkDistance);
            preferences.putFloat("placeForkDist", webPlaceForkDistance);
            
            request->send(200, "text/plain", "Config Saved");
            Serial.println("=== WEB CONFIG SAVED ===");
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });

    server.begin();
    Serial.println("Web Server initialized");
    Serial.println("Sensor Dashboard available at /sensors");
}

void initializeWebServer() {
    initWebServer();
}

void updateWebServer() {
    // Web server handles requests asynchronously, no update needed
}

bool isWebMoveRequested() {
    return webMoveRequested;
}

void clearWebMoveRequest() {
    webMoveRequested = false;
}

bool isWebStartRequested() {
    return webStartRequested;
}

void clearWebStartRequest() {
    webStartRequested = false;
}

bool isWebTestPositionRequested() {
    return webTestPositionRequested;
}

void clearWebTestPositionRequest() {
    webTestPositionRequested = false;
}

float getWebTargetX() { return webTargetX; }
float getWebTargetY() { return webTargetY; }

float getWebPickX() { return webPickX; }
float getWebPickY() { return webPickY; }
float getWebPlaceX() { return webPlaceX; }
float getWebPlaceY() { return webPlaceY; }
float getWebPickForkDistance() { return webPickForkDistance; }
float getWebPlaceForkDistance() { return webPlaceForkDistance; }
