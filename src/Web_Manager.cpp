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
<!DOCTYPE HTML><html>
<head>
  <title>Paint Machine Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background-color: #f0f0f0; }
    .container { max-width: 500px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    h1 { color: #333; }
    h2 { color: #666; border-bottom: 1px solid #eee; padding-bottom: 10px; margin-top: 30px; }
    .input-group { margin-bottom: 15px; text-align: left; }
    .row { display: flex; gap: 10px; }
    .col { flex: 1; }
    label { display: block; margin-bottom: 5px; font-weight: bold; font-size: 0.9em; }
    input[type="number"] { width: 100%; padding: 8px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
    button { background-color: #4CAF50; color: white; padding: 12px 24px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border: none; border-radius: 5px; width: 100%; transition: background 0.3s; }
    button:hover { background-color: #45a049; }
    button.secondary { background-color: #2196F3; }
    button.secondary:hover { background-color: #0b7dda; }
    .status { margin-top: 20px; padding: 10px; background-color: #e7f3fe; border-left: 6px solid #2196F3; display: none; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Paint Machine Control</h1>
    
    <!-- Pick and Place Configuration -->
    <h2>Pick & Place Configuration</h2>
    <div class="row">
        <div class="col">
            <div class="input-group">
                <label>Pick X (in)</label>
                <input type="number" id="pickX" step="0.1">
            </div>
        </div>
        <div class="col">
             <div class="input-group">
                <label>Pick Y (in)</label>
                <input type="number" id="pickY" step="0.1">
            </div>
        </div>
    </div>
    <div class="row">
        <div class="col">
            <div class="input-group">
                <label>Place X (in)</label>
                <input type="number" id="placeX" step="0.1">
            </div>
        </div>
        <div class="col">
             <div class="input-group">
                <label>Place Y (in)</label>
                <input type="number" id="placeY" step="0.1">
            </div>
        </div>
    </div>
    <div class="input-group">
        <label>Fork Distance (in)</label>
        <input type="number" id="forkDist" step="0.1">
    </div>
    <button class="secondary" onclick="saveConfig()">SAVE CONFIGURATION</button>
    <p><small>Use physical START button to run sequence</small></p>

    <!-- Direct Move Control -->
    <h2>Direct Move</h2>
    <div class="row">
        <div class="col">
            <div class="input-group">
                <label>Target X</label>
                <input type="number" id="moveX" step="0.1">
            </div>
        </div>
        <div class="col">
             <div class="input-group">
                <label>Target Y</label>
                <input type="number" id="moveY" step="0.1">
            </div>
        </div>
    </div>
    <button onclick="sendMove()">MOVE NOW</button>
    
    <div id="status" class="status"></div>
  </div>

  <script>
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
        alert("Please enter both X and Y coordinates");
        return;
      }
      
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/move?x=" + x + "&y=" + y, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          showStatus("Move command sent: X=" + x + ", Y=" + y);
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
        alert("Please fill in all Pick & Place fields");
        return;
      }

      var url = "/config?px=" + px + "&py=" + py + "&plx=" + plx + "&ply=" + ply + "&fd=" + fd;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", url, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          showStatus("Configuration Saved!");
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
