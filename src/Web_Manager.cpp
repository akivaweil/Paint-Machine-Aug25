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

// Move request variables
volatile bool webMoveRequested = false;
volatile float webTargetX = 0.0;
volatile float webTargetY = 0.0;

// HTML Content
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Paint Machine Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; background-color: #f0f0f0; }
    .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    h1 { color: #333; }
    .input-group { margin-bottom: 15px; text-align: left; }
    label { display: block; margin-bottom: 5px; font-weight: bold; }
    input[type="number"] { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
    button { background-color: #4CAF50; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; border: none; border-radius: 5px; width: 100%; }
    button:hover { background-color: #45a049; }
    .status { margin-top: 20px; padding: 10px; background-color: #e7f3fe; border-left: 6px solid #2196F3; display: none; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Paint Machine Control</h1>
    <div class="input-group">
      <label for="x">X Coordinate (inches):</label>
      <input type="number" id="x" step="0.1" placeholder="Enter X">
    </div>
    <div class="input-group">
      <label for="y">Y Coordinate (inches):</label>
      <input type="number" id="y" step="0.1" placeholder="Enter Y">
    </div>
    <button onclick="sendMove()">MOVE</button>
    <div id="status" class="status"></div>
  </div>

  <script>
    function sendMove() {
      var x = document.getElementById("x").value;
      var y = document.getElementById("y").value;
      
      if(x === "" || y === "") {
        alert("Please enter both X and Y coordinates");
        return;
      }
      
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/move?x=" + x + "&y=" + y, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          var statusDiv = document.getElementById("status");
          statusDiv.style.display = "block";
          statusDiv.innerHTML = "Move command sent: X=" + x + ", Y=" + y;
          setTimeout(function() { statusDiv.style.display = "none"; }, 3000);
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

    server.begin();
    Serial.println("Web Server initialized");
}

bool isWebMoveRequested() {
    return webMoveRequested;
}

void clearWebMoveRequest() {
    webMoveRequested = false;
}

float getWebTargetX() {
    return webTargetX;
}

float getWebTargetY() {
    return webTargetY;
}

