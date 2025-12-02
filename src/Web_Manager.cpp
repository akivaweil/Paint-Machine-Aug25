#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Web_Manager.h"
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"
#include "StateMachine/STATES/01_HOMING.h"

//* ************************************************************************
//* ************************ WEB MANAGER ***********************************
//* ************************************************************************

// Web server instance
AsyncWebServer server(80);

// Sensor pins initialized flag
bool sensorsInitialized = false;

// Motor instances
StepperMotor* motorX = nullptr;
StepperMotor* motorY = nullptr;
StepperMotor* motorFork = nullptr;

// Home switch instances
HomeSwitch* homeSwitchX = nullptr;
HomeSwitch* homeSwitchY = nullptr;
HomeSwitch* homeSwitchFork = nullptr;

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
      display: flex;
      flex-direction: column;
      gap: 12px;
      margin-bottom: 30px;
      width: 20%;
      min-width: 180px;
      align-self: flex-start;
    }
    
    .sensor-card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: var(--border-radius);
      padding: 16px;
      box-shadow: var(--shadow);
      transition: all 0.3s ease;
      position: relative;
      overflow: hidden;
      width: 100%;
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
    
    .control-panel {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: var(--border-radius);
      padding: 24px;
      box-shadow: var(--shadow);
      margin-top: 30px;
    }
    
    .control-panel h2 {
      font-size: 1.2rem;
      font-weight: 600;
      color: var(--text-primary);
      margin-bottom: 20px;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    
    .arrow-controls {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 12px;
      max-width: 300px;
      margin: 0 auto;
    }
    
    .arrow-btn {
      background: var(--card-border);
      border: 2px solid var(--card-border);
      border-radius: 8px;
      padding: 20px;
      font-size: 1.5rem;
      color: var(--text-primary);
      cursor: pointer;
      transition: all 0.2s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      user-select: none;
    }
    
    .arrow-btn:hover {
      background: var(--accent-blue);
      border-color: var(--accent-blue);
      transform: scale(1.05);
    }
    
    .arrow-btn:active {
      transform: scale(0.95);
    }
    
    .arrow-btn.up {
      grid-column: 2;
    }
    
    .arrow-btn.down {
      grid-column: 2;
      grid-row: 2;
    }
    
    .arrow-btn.left {
      grid-column: 1;
      grid-row: 2;
    }
    
    .arrow-btn.right {
      grid-column: 3;
      grid-row: 2;
    }
    
    .distance-selector {
      display: flex;
      gap: 10px;
      justify-content: center;
      margin-top: 20px;
    }
    
    .distance-btn {
      background: var(--card-border);
      border: 2px solid var(--card-border);
      border-radius: 8px;
      padding: 10px 20px;
      color: var(--text-primary);
      cursor: pointer;
      transition: all 0.2s ease;
      font-size: 0.9rem;
      font-weight: 500;
    }
    
    .distance-btn.active {
      background: var(--accent-green);
      border-color: var(--accent-green);
    }
    
    .distance-btn:hover {
      border-color: var(--accent-blue);
    }
    
    .axis-label {
      text-align: center;
      margin-top: 15px;
      color: var(--text-secondary);
      font-size: 0.85rem;
    }
    
    .home-buttons {
      display: flex;
      gap: 10px;
      justify-content: center;
      margin-top: 20px;
      flex-wrap: wrap;
    }
    
    .home-btn {
      background: var(--card-border);
      border: 2px solid var(--card-border);
      border-radius: 8px;
      padding: 12px 20px;
      color: var(--text-primary);
      cursor: pointer;
      transition: all 0.2s ease;
      font-size: 0.9rem;
      font-weight: 500;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    
    .home-btn:hover {
      background: var(--accent-yellow);
      border-color: var(--accent-yellow);
      transform: scale(1.05);
    }
    
    .home-btn:active {
      transform: scale(0.95);
    }
    
    .home-btn.all {
      background: var(--accent-blue);
      border-color: var(--accent-blue);
    }
    
    .home-btn.all:hover {
      background: var(--accent-green);
      border-color: var(--accent-green);
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
    
    <div class="control-panel">
      <h2>Manual Controls</h2>
      <div style="display: grid; grid-template-columns: 1fr 1fr; gap: 30px; margin-top: 20px;">
        <div>
          <div class="axis-label">Gantry (X/Y)</div>
          <div class="arrow-controls" style="max-width: 250px;">
            <button class="arrow-btn up" id="btnUp" onmousedown="moveY(-1)" onmouseup="stopMove()" ontouchstart="moveY(-1)" ontouchend="stopMove()">&uarr;</button>
            <button class="arrow-btn left" id="btnLeft" onmousedown="moveX(-1)" onmouseup="stopMove()" ontouchstart="moveX(-1)" ontouchend="stopMove()">&larr;</button>
            <button class="arrow-btn right" id="btnRight" onmousedown="moveX(1)" onmouseup="stopMove()" ontouchstart="moveX(1)" ontouchend="stopMove()">&rarr;</button>
            <button class="arrow-btn down" id="btnDown" onmousedown="moveY(1)" onmouseup="stopMove()" ontouchstart="moveY(1)" ontouchend="stopMove()">&darr;</button>
          </div>
          <div class="distance-selector" style="margin-top: 15px;">
            <button class="distance-btn active" id="btn1in" onclick="setDistance(1)">1"</button>
            <button class="distance-btn" id="btn3in" onclick="setDistance(3)">3"</button>
          </div>
        </div>
        <div>
          <div class="axis-label">Fork</div>
          <div class="arrow-controls" style="max-width: 150px;">
            <button class="arrow-btn up" id="btnForkUp" onmousedown="moveFork(-1)" onmouseup="stopMove()" ontouchstart="moveFork(-1)" ontouchend="stopMove()">&uarr;</button>
            <button class="arrow-btn down" id="btnForkDown" onmousedown="moveFork(1)" onmouseup="stopMove()" ontouchstart="moveFork(1)" ontouchend="stopMove()">&darr;</button>
          </div>
          <div class="distance-selector" style="margin-top: 15px;">
            <button class="distance-btn active" id="btnFork1in" onclick="setForkDistance(1)">1"</button>
            <button class="distance-btn" id="btnFork3in" onclick="setForkDistance(3)">3"</button>
          </div>
        </div>
      </div>
      <div class="home-buttons">
        <button class="home-btn" onclick="homeAxis('x')">Home X</button>
        <button class="home-btn" onclick="homeAxis('y')">Home Y</button>
        <button class="home-btn" onclick="homeAxis('fork')">Home Fork</button>
        <button class="home-btn all" onclick="homeAxis('all')">Home All</button>
      </div>
    </div>
    
    <div class="last-update" id="lastUpdate">Last update: --</div>
    
    <div class="footer">
      Paint Machine Control System
    </div>
  </div>
  
  <script>
    const sensors = [
      { id: 'xHome1', name: 'X Home Switch 1', pin: 'Pin 7' },
      { id: 'xHome2', name: 'X Home Switch 2', pin: 'Pin 6' },
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
    
    // Movement control
    let moveDistance = 1; // Default 1 inch
    let forkDistance = 1; // Default 1 inch
    
    function setDistance(inches) {
      moveDistance = inches;
      document.getElementById('btn1in').classList.toggle('active', inches === 1);
      document.getElementById('btn3in').classList.toggle('active', inches === 3);
    }
    
    function setForkDistance(inches) {
      forkDistance = inches;
      document.getElementById('btnFork1in').classList.toggle('active', inches === 1);
      document.getElementById('btnFork3in').classList.toggle('active', inches === 3);
    }
    
    function moveX(direction) {
      const distance = direction * moveDistance;
      fetch('/api/move?axis=x&distance=' + distance)
        .catch(error => console.error('Move error:', error));
    }
    
    function moveY(direction) {
      const distance = direction * moveDistance;
      fetch('/api/move?axis=y&distance=' + distance)
        .catch(error => console.error('Move error:', error));
    }
    
    function moveFork(direction) {
      const distance = direction * forkDistance;
      fetch('/api/move?axis=fork&distance=' + distance)
        .catch(error => console.error('Move error:', error));
    }
    
    function stopMove() {
      // Movement stops when button is released
      // The server handles the movement as a single command
    }
    
    function homeAxis(axis) {
      fetch('/api/home?axis=' + axis)
        .then(response => response.text())
        .then(data => {
          console.log('Home response:', data);
        })
        .catch(error => console.error('Home error:', error));
    }
    
    // Keyboard controls
    document.addEventListener('keydown', function(e) {
      if (e.key === 'ArrowUp') {
        e.preventDefault();
        moveY(-1);
      } else if (e.key === 'ArrowDown') {
        e.preventDefault();
        moveY(1);
      } else if (e.key === 'ArrowLeft') {
        e.preventDefault();
        moveX(-1);
      } else if (e.key === 'ArrowRight') {
        e.preventDefault();
        moveX(1);
      }
    });
  </script>
</body>
</html>
)rawliteral";

// Initialize sensor pins
void initializeSensors() {
    if (sensorsInitialized) return;
    
    // Initialize test button (assuming active LOW with pullup, adjust if needed)
    pinMode(TEST_BUTTON_PIN, INPUT_PULLUP);
    
    // Initialize HomeSwitch instances (pins configured in begin())
    if (homeSwitchX == nullptr) {
        homeSwitchX = new HomeSwitch(X_HOME_PIN, X_HOME_PIN2);
        homeSwitchX->begin();
    }
    if (homeSwitchY == nullptr) {
        homeSwitchY = new HomeSwitch(Y_HOME_PIN);
        homeSwitchY->begin();
    }
    if (homeSwitchFork == nullptr) {
        homeSwitchFork = new HomeSwitch(FORK_HOME_PIN);
        homeSwitchFork->begin();
    }
    
    sensorsInitialized = true;
}

// Initialize motors
void initializeMotors() {
    if (motorX == nullptr) {
        motorX = new StepperMotor(X_STEP_PIN, X_DIR_PIN, X_STEPS_PER_INCH, X_MAX_SPEED, X_MAX_ACCEL);
    }
    if (motorY == nullptr) {
        motorY = new StepperMotor(Y_STEP_PIN, Y_DIR_PIN, STEPS_PER_INCH, Y_MAX_SPEED, Y_MAX_ACCEL);
    }
    if (motorFork == nullptr) {
        motorFork = new StepperMotor(FORK_STEP_PIN, FORK_DIR_PIN, STEPS_PER_INCH, FORK_MAX_SPEED, FORK_MAX_ACCEL);
    }
}


// Read sensor states
String getSensorStatesJSON() {
    // Read X home switches (need individual pins for JSON)
    bool xHome1 = homeSwitchX ? digitalRead(X_HOME_PIN) : false;
    bool xHome2 = homeSwitchX ? digitalRead(X_HOME_PIN2) : false;
    // Read Y and Fork using HomeSwitch instances
    bool yHome = homeSwitchY ? homeSwitchY->read() : false;
    bool forkHome = homeSwitchFork ? homeSwitchFork->read() : false;
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

void initializeWebServer() {
    // Initialize sensor pins
    initializeSensors();
    
    // Initialize motors
    initializeMotors();

    // Route for root / web page (sensor dashboard)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", sensors_html);
    });
    
    // API endpoint for sensor states
    server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorStatesJSON();
        request->send(200, "application/json", json);
    });
    
    // API endpoint for movement
    server.on("/api/move", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("axis") && request->hasParam("distance")) {
            String axis = request->getParam("axis")->value();
            float distance = request->getParam("distance")->value().toFloat();
            
            StepperMotor* motor = nullptr;
            const char* axisName = "";
            
            if (axis == "x") {
                motor = motorX;
                axisName = "X";
            } else if (axis == "y") {
                motor = motorY;
                axisName = "Y";
            } else if (axis == "fork") {
                motor = motorFork;
                axisName = "Fork";
            }
            
            if (motor) {
                motor->moveInches(distance);
                request->send(200, "text/plain", "OK");
                Serial.printf("Web Request: Move %s by %.2f inches\n", axisName, distance);
            } else {
                request->send(400, "text/plain", "Invalid axis or motor not initialized");
            }
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });
    
    // API endpoint for homing
    server.on("/api/home", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("axis")) {
            String axis = request->getParam("axis")->value();
            
            if (axis == "x") {
                homeXAxis();
                request->send(200, "text/plain", "OK");
                Serial.println("Web Request: Home X axis");
            } else if (axis == "y") {
                homeYAxis();
                request->send(200, "text/plain", "OK");
                Serial.println("Web Request: Home Y axis");
            } else if (axis == "fork") {
                homeForkAxis();
                request->send(200, "text/plain", "OK");
                Serial.println("Web Request: Home Fork axis");
            } else if (axis == "all") {
                homeAllAxes();
                request->send(200, "text/plain", "OK");
                Serial.println("Web Request: Home all axes");
            } else {
                request->send(400, "text/plain", "Invalid axis");
            }
        } else {
            request->send(400, "text/plain", "Missing axis parameter");
        }
    });

    server.begin();
    Serial.println("Web Server initialized");
    Serial.println("Sensor Dashboard available at /");
}


void updateWebServer() {
    // Web server handles requests asynchronously, no update needed
}
