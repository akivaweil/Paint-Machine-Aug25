#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "Web_Manager.h"
#include "config/Config.h"

//* ************************************************************************
//* ************************ WEB MANAGER ***********************************
//* ************************************************************************

// Web server instance
AsyncWebServer server(80);

// Move request variables (Direct Move)
volatile bool webMoveRequested = false;
volatile float webTargetX = 0.0;
volatile float webTargetY = 0.0;

// Pick and Place Sequence Variables
volatile float webPickX = 0.0;
volatile float webPickY = 0.0;
volatile float webPlaceX = 0.0;
volatile float webPlaceY = 0.0;
volatile float webForkDistance = 0.0;

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
      --primary-color: #2563eb;
      --secondary-color: #475569;
      --accent-color: #10b981;
      --bg-color: #f1f5f9;
      --card-bg: #ffffff;
      --text-color: #1e293b;
      --border-radius: 16px;
      --shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.1), 0 2px 4px -1px rgba(0, 0, 0, 0.06);
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
    }
    
    .card:hover {
      transform: translateY(-2px);
      box-shadow: 0 10px 15px -3px rgba(0, 0, 0, 0.1), 0 4px 6px -2px rgba(0, 0, 0, 0.05);
    }

    .card h2 {
      margin-top: 0;
      margin-bottom: 25px;
      font-size: 1.25rem;
      color: var(--secondary-color);
      border-bottom: 2px solid #f1f5f9;
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
      border: 2px solid #e2e8f0;
      border-radius: 10px;
      font-size: 1.1rem;
      transition: border-color 0.3s;
      box-sizing: border-box;
      background: #f8fafc;
    }

    input[type="number"]:focus {
      border-color: var(--primary-color);
      outline: none;
      background: #fff;
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
    }

    .btn:active {
      transform: scale(0.98);
    }

    .btn-primary {
      background-color: var(--primary-color);
      color: white;
      box-shadow: 0 4px 6px -1px rgba(37, 99, 235, 0.2);
    }
    
    .btn-primary:hover {
      background-color: #1d4ed8;
      box-shadow: 0 4px 12px rgba(37, 99, 235, 0.3);
    }

    .btn-success {
      background-color: var(--accent-color);
      color: white;
      box-shadow: 0 4px 6px -1px rgba(16, 185, 129, 0.2);
    }
    
    .btn-success:hover {
      background-color: #059669;
      box-shadow: 0 4px 12px rgba(16, 185, 129, 0.3);
    }

    .full-width {
      grid-column: 1 / -1;
    }

    .status-bar {
      position: fixed;
      bottom: 30px;
      left: 50%;
      transform: translateX(-50%);
      background-color: #1e293b;
      color: white;
      padding: 16px 32px;
      border-radius: 50px;
      box-shadow: 0 10px 25px -5px rgba(0, 0, 0, 0.3);
      display: none;
      z-index: 1000;
      font-weight: 500;
      letter-spacing: 0.5px;
      min-width: 200px;
      text-align: center;
    }

    .row {
      display: flex;
      gap: 20px;
    }
    
    .col {
      flex: 1;
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
<body>
  <div class="header">
    <h1>Paint Machine</h1>
    <p>Control Dashboard</p>
  </div>

  <div class="grid-container">
    <!-- Pick Card -->
    <div class="card">
      <h2>Pick Position</h2>
      <div class="input-group">
        <label for="pickX">X Coordinate (inches)</label>
        <input type="number" id="pickX" step="0.1" placeholder="0.0">
      </div>
      <div class="input-group">
        <label for="pickY">Y Coordinate (inches)</label>
        <input type="number" id="pickY" step="0.1" placeholder="0.0">
      </div>
    </div>

    <!-- Place Card -->
    <div class="card">
      <h2>Place Position</h2>
      <div class="input-group">
        <label for="placeX">X Coordinate (inches)</label>
        <input type="number" id="placeX" step="0.1" placeholder="0.0">
      </div>
      <div class="input-group">
        <label for="placeY">Y Coordinate (inches)</label>
        <input type="number" id="placeY" step="0.1" placeholder="0.0">
      </div>
    </div>

    <!-- Settings Card -->
    <div class="card">
      <h2>Sequence Settings</h2>
      <div class="input-group">
        <label for="forkDist">Fork Extension (inches)</label>
        <input type="number" id="forkDist" step="0.1" placeholder="0.0">
      </div>
      <button class="btn btn-success" onclick="saveConfig()">Save Configuration</button>
      <p style="font-size: 0.85rem; color: #94a3b8; margin-top: 15px; text-align: center; margin-bottom: 0;">
        Use physical START button to run sequence
      </p>
    </div>

    <!-- Direct Move Card -->
    <div class="card full-width">
      <h2 style="border-bottom-color: #dbeafe;">
        <span style="background-color: var(--primary-color); width: 6px; height: 24px; border-radius: 3px; margin-right: 12px;"></span>
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
    // Load values from ESP32 if possible, or persist locally
    // For now we rely on user inputting them or browser cache
    
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
      var fd = document.getElementById("forkDist").value;

      if(px === "" || py === "" || plx === "" || ply === "" || fd === "") {
        showStatus("⚠️ Please fill in all Pick & Place fields");
        return;
      }

      var url = "/config?px=" + px + "&py=" + py + "&plx=" + plx + "&ply=" + ply + "&fd=" + fd;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", url, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          showStatus("✅ Configuration Saved!");
        }
      };
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

void initWebServer() {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
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

    // Route to handle configuration
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("px") && request->hasParam("py") && 
            request->hasParam("plx") && request->hasParam("ply") && 
            request->hasParam("fd")) {
            
            webPickX = request->getParam("px")->value().toFloat();
            webPickY = request->getParam("py")->value().toFloat();
            webPlaceX = request->getParam("plx")->value().toFloat();
            webPlaceY = request->getParam("ply")->value().toFloat();
            webForkDistance = request->getParam("fd")->value().toFloat();
            
            request->send(200, "text/plain", "Config Saved");
            Serial.println("=== WEB CONFIG UPDATED ===");
            Serial.print("Pick: "); Serial.print(webPickX); Serial.print(", "); Serial.println(webPickY);
            Serial.print("Place: "); Serial.print(webPlaceX); Serial.print(", "); Serial.println(webPlaceY);
            Serial.print("Fork Dist: "); Serial.println(webForkDistance);
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });

    server.begin();
    Serial.println("Web Server initialized");
}

bool isWebMoveRequested() {
    return webMoveRequested;
}

void clearWebMoveRequest() {
    webMoveRequested = false;
}

float getWebTargetX() { return webTargetX; }
float getWebTargetY() { return webTargetY; }

float getWebPickX() { return webPickX; }
float getWebPickY() { return webPickY; }
float getWebPlaceX() { return webPlaceX; }
float getWebPlaceY() { return webPlaceY; }
float getWebForkDistance() { return webForkDistance; }
