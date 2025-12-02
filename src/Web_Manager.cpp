#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include "Web_Manager.h"
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/STATES/02_TEST.h"

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
StepperMotor* motorStorage = nullptr;

// Home switch instances
HomeSwitch* homeSwitchX = nullptr;
HomeSwitch* homeSwitchY = nullptr;
HomeSwitch* homeSwitchFork = nullptr;

// Test position values
float testPos1X = 0.0;
float testPos1Y = 0.0;
float testPos1Fork = 0.0;
float testPos2X = 0.0;
float testPos2Y = 0.0;
float testPos2Fork = 0.0;

// Motor speed and acceleration settings
long motorSpeedX = X_MAX_SPEED;
long motorSpeedY = Y_MAX_SPEED;
long motorSpeedFork = FORK_MAX_SPEED;
long motorAccelX = X_MAX_ACCEL;
long motorAccelY = Y_MAX_ACCEL;
long motorAccelFork = FORK_MAX_ACCEL;

// Preferences namespace for test sequence persistence
Preferences preferences;

// Save test position values to non-volatile storage
void saveTestPositions() {
    preferences.begin("testSeq", false);
    preferences.putFloat("pos1X", testPos1X);
    preferences.putFloat("pos1Y", testPos1Y);
    preferences.putFloat("pos1Fork", testPos1Fork);
    preferences.putFloat("pos2X", testPos2X);
    preferences.putFloat("pos2Y", testPos2Y);
    preferences.putFloat("pos2Fork", testPos2Fork);
    preferences.end();
}

// Load test position values from non-volatile storage
void loadTestPositions() {
    preferences.begin("testSeq", true);
    testPos1X = preferences.getFloat("pos1X", 0.0);
    testPos1Y = preferences.getFloat("pos1Y", 0.0);
    testPos1Fork = preferences.getFloat("pos1Fork", 0.0);
    testPos2X = preferences.getFloat("pos2X", 0.0);
    testPos2Y = preferences.getFloat("pos2Y", 0.0);
    testPos2Fork = preferences.getFloat("pos2Fork", 0.0);
    preferences.end();
}

// Save motor settings to non-volatile storage
void saveMotorSettings() {
    preferences.begin("motor", false);
    preferences.putLong("speedX", motorSpeedX);
    preferences.putLong("speedY", motorSpeedY);
    preferences.putLong("speedFork", motorSpeedFork);
    preferences.putLong("accelX", motorAccelX);
    preferences.putLong("accelY", motorAccelY);
    preferences.putLong("accelFork", motorAccelFork);
    preferences.end();
}

// Load motor settings from non-volatile storage
void loadMotorSettings() {
    preferences.begin("motor", true);
    motorSpeedX = preferences.getLong("speedX", X_MAX_SPEED);
    motorSpeedY = preferences.getLong("speedY", Y_MAX_SPEED);
    motorSpeedFork = preferences.getLong("speedFork", FORK_MAX_SPEED);
    motorAccelX = preferences.getLong("accelX", X_MAX_ACCEL);
    motorAccelY = preferences.getLong("accelY", Y_MAX_ACCEL);
    motorAccelFork = preferences.getLong("accelFork", FORK_MAX_ACCEL);
    preferences.end();
}

// Apply motor settings to motors
void applyMotorSettings() {
    if (motorX) {
        motorX->setSpeed(motorSpeedX);
        motorX->setAcceleration(motorAccelX);
    }
    if (motorY) {
        motorY->setSpeed(motorSpeedY);
        motorY->setAcceleration(motorAccelY);
    }
    if (motorFork) {
        motorFork->setSpeed(motorSpeedFork);
        motorFork->setAcceleration(motorAccelFork);
    }
}

// Sensor Dashboard HTML
const char sensors_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
  <title>Paint Machine</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;500;600;700&family=Outfit:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg-deep: #0d0d0d;
      --bg-surface: #161616;
      --bg-elevated: #1f1f1f;
      --bg-card: #252525;
      --border-subtle: #333;
      --border-accent: #444;
      --text-primary: #fafafa;
      --text-secondary: #888;
      --text-muted: #555;
      --accent-primary: #ff6b35;
      --accent-secondary: #00d4aa;
      --accent-warning: #ffcc00;
      --accent-danger: #ff4757;
      --accent-glow: rgba(255, 107, 53, 0.4);
      --accent-green-glow: rgba(0, 212, 170, 0.4);
      --radius-sm: 6px;
      --radius-md: 10px;
      --radius-lg: 16px;
    }
    
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: 'Outfit', sans-serif;
      background: var(--bg-deep);
      color: var(--text-primary);
      min-height: 100vh;
      position: relative;
      overflow-x: hidden;
    }
    
    body::before {
      content: '';
      position: fixed;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background: 
        radial-gradient(ellipse 80% 50% at 50% -20%, rgba(255, 107, 53, 0.08) 0%, transparent 50%),
        radial-gradient(ellipse 60% 40% at 100% 100%, rgba(0, 212, 170, 0.05) 0%, transparent 40%),
        repeating-linear-gradient(0deg, transparent, transparent 100px, rgba(255,255,255,0.01) 100px, rgba(255,255,255,0.01) 101px);
      pointer-events: none;
      z-index: 0;
    }
    
    .container {
      max-width: 1100px;
      margin: 0 auto;
      padding: 30px 20px;
      position: relative;
      z-index: 1;
    }
    
    .header {
      text-align: center;
      margin-bottom: 50px;
      padding: 20px 0;
      position: relative;
    }
    
    .header::after {
      content: '';
      position: absolute;
      bottom: 0;
      left: 50%;
      transform: translateX(-50%);
      width: 120px;
      height: 2px;
      background: linear-gradient(90deg, transparent, var(--accent-primary), transparent);
    }
    
    .header h1 {
      font-family: 'JetBrains Mono', monospace;
      font-size: 2.2rem;
      font-weight: 700;
      color: var(--text-primary);
      letter-spacing: 4px;
      text-transform: uppercase;
      margin-bottom: 8px;
    }
    
    .header h1 span {
      color: var(--accent-primary);
    }
    
    .header p {
      font-family: 'JetBrains Mono', monospace;
      color: var(--text-muted);
      font-size: 0.75rem;
      letter-spacing: 3px;
      text-transform: uppercase;
    }
    
    .main-layout {
      display: grid;
      grid-template-columns: 200px 1fr;
      gap: 30px;
      align-items: start;
    }
    
    .sensor-panel {
      position: sticky;
      top: 30px;
    }
    
    .panel-label {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.65rem;
      color: var(--text-muted);
      letter-spacing: 2px;
      text-transform: uppercase;
      margin-bottom: 12px;
      padding-left: 4px;
    }
    
    .sensor-grid {
      display: flex;
      flex-direction: column;
      gap: 8px;
    }
    
    .sensor-card {
      background: var(--bg-surface);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-md);
      padding: 12px 14px;
      transition: all 0.25s cubic-bezier(0.4, 0, 0.2, 1);
      position: relative;
      overflow: hidden;
    }
    
    .sensor-card::before {
      content: '';
      position: absolute;
      left: 0;
      top: 0;
      bottom: 0;
      width: 3px;
      background: var(--border-subtle);
      transition: all 0.25s ease;
    }
    
    .sensor-card.active {
      border-color: var(--accent-secondary);
      background: linear-gradient(135deg, var(--bg-surface) 0%, rgba(0, 212, 170, 0.05) 100%);
    }
    
    .sensor-card.active::before {
      background: var(--accent-secondary);
      box-shadow: 0 0 12px var(--accent-green-glow);
    }
    
    .sensor-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    
    .sensor-name {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.7rem;
      font-weight: 500;
      color: var(--text-secondary);
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    
    .sensor-card.active .sensor-name {
      color: var(--text-primary);
    }
    
    .status-indicator {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: var(--text-muted);
      transition: all 0.25s ease;
    }
    
    .sensor-card.active .status-indicator {
      background: var(--accent-secondary);
      box-shadow: 0 0 8px var(--accent-green-glow), 0 0 16px var(--accent-green-glow);
      animation: glow 1.5s ease-in-out infinite alternate;
    }
    
    @keyframes glow {
      from { box-shadow: 0 0 8px var(--accent-green-glow), 0 0 16px var(--accent-green-glow); }
      to { box-shadow: 0 0 12px var(--accent-green-glow), 0 0 24px var(--accent-green-glow); }
    }
    
    .content-area {
      display: flex;
      flex-direction: column;
      gap: 24px;
      }
    
    .card {
      background: var(--bg-surface);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-lg);
      overflow: hidden;
    }
    
    .card-header {
      padding: 16px 20px;
      border-bottom: 1px solid var(--border-subtle);
      display: flex;
      align-items: center;
      gap: 12px;
    }
    
    .card-icon {
      width: 32px;
      height: 32px;
      background: linear-gradient(135deg, var(--accent-primary) 0%, #ff8f5a 100%);
      border-radius: var(--radius-sm);
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 1rem;
    }
    
    .card-title {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.8rem;
      font-weight: 600;
      color: var(--text-primary);
      letter-spacing: 1px;
      text-transform: uppercase;
    }
    
    .card-body {
      padding: 24px;
    }
    
    .controls-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 40px;
    }
    
    .control-section {
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    
    .control-label {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.7rem;
      color: var(--text-muted);
      letter-spacing: 2px;
      text-transform: uppercase;
      margin-bottom: 16px;
    }
    
    .position-display {
      display: flex;
      gap: 16px;
      margin-bottom: 20px;
      font-family: 'JetBrains Mono', monospace;
    }
    
    .pos-item {
      display: flex;
      align-items: baseline;
      gap: 6px;
    }
    
    .pos-label {
      font-size: 0.7rem;
      color: var(--text-muted);
    }
    
    .pos-value {
      font-size: 1.1rem;
      font-weight: 600;
      color: var(--accent-primary);
    }
    
    .arrow-controls {
      display: grid;
      grid-template-columns: repeat(3, 48px);
      grid-template-rows: repeat(2, 48px);
      gap: 6px;
    }
    
    .arrow-btn {
      background: var(--bg-elevated);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-sm);
      font-size: 1.2rem;
      color: var(--text-secondary);
      cursor: pointer;
      transition: all 0.15s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      user-select: none;
    }
    
    .arrow-btn:hover {
      background: var(--accent-primary);
      border-color: var(--accent-primary);
      color: var(--text-primary);
      transform: scale(1.05);
      box-shadow: 0 4px 20px var(--accent-glow);
    }
    
    .arrow-btn:active {
      transform: scale(0.95);
    }
    
    .arrow-btn.up { grid-column: 2; }
    .arrow-btn.down { grid-column: 2; grid-row: 2; }
    .arrow-btn.left { grid-column: 1; grid-row: 2; }
    .arrow-btn.right { grid-column: 3; grid-row: 2; }
    
    .distance-selector {
      display: flex;
      gap: 8px;
      margin-top: 16px;
    }
    
    .distance-btn {
      background: var(--bg-elevated);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-sm);
      padding: 8px 16px;
      color: var(--text-secondary);
      cursor: pointer;
      transition: all 0.15s ease;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.75rem;
      font-weight: 500;
    }
    
    .distance-btn.active {
      background: var(--accent-secondary);
      border-color: var(--accent-secondary);
      color: var(--bg-deep);
    }
    
    .distance-btn:hover:not(.active) {
      border-color: var(--accent-secondary);
      color: var(--accent-secondary);
    }
    
    .home-buttons {
      display: flex;
      gap: 10px;
      justify-content: center;
      padding-top: 24px;
      border-top: 1px solid var(--border-subtle);
      margin-top: 24px;
    }
    
    .home-btn {
      background: var(--bg-elevated);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-sm);
      padding: 10px 18px;
      color: var(--text-secondary);
      cursor: pointer;
      transition: all 0.15s ease;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.7rem;
      font-weight: 500;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    
    .home-btn:hover {
      background: var(--accent-warning);
      border-color: var(--accent-warning);
      color: var(--bg-deep);
      transform: translateY(-2px);
    }
    
    .home-btn.all {
      background: var(--accent-primary);
      border-color: var(--accent-primary);
      color: white;
    }
    
    .home-btn.all:hover {
      background: var(--accent-secondary);
      border-color: var(--accent-secondary);
      box-shadow: 0 4px 20px var(--accent-green-glow);
    }
    
    .collapsible-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
      cursor: pointer;
      user-select: none;
      padding: 16px 20px;
      border-bottom: 1px solid transparent;
      transition: all 0.2s ease;
    }
    
    .collapsible-header:hover {
      background: var(--bg-elevated);
    }
    
    .collapsible-header.active {
      border-bottom-color: var(--border-subtle);
    }
    
    .collapsible-header .card-header-content {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    
    .chevron {
      transition: transform 0.3s ease;
      font-size: 0.8rem;
      color: var(--text-muted);
    }
    
    .collapsible-header.active .chevron {
      transform: rotate(180deg);
    }
    
    .collapsible-content {
      max-height: 0;
      overflow: hidden;
      transition: max-height 0.4s cubic-bezier(0.4, 0, 0.2, 1);
    }
    
    .collapsible-content.expanded {
      max-height: 1000px;
    }
    
    .position-group {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
      margin-bottom: 20px;
    }
    
    .position-inputs {
      background: var(--bg-elevated);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-md);
      padding: 16px;
    }
    
    .position-inputs h3 {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.7rem;
      font-weight: 600;
      color: var(--accent-primary);
      margin-bottom: 14px;
      text-align: center;
      letter-spacing: 1px;
      text-transform: uppercase;
    }
    
    .input-row {
      display: flex;
      align-items: center;
      gap: 10px;
      margin-bottom: 10px;
    }
    
    .input-row:last-child {
      margin-bottom: 0;
    }
    
    .input-row label {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.7rem;
      color: var(--text-muted);
      min-width: 50px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }
    
    .input-row input {
      flex: 1;
      background: var(--bg-surface);
      border: 1px solid var(--border-subtle);
      border-radius: var(--radius-sm);
      padding: 10px 12px;
      color: var(--text-primary);
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.85rem;
      transition: all 0.2s ease;
    }
    
    .input-row input:focus {
      outline: none;
      border-color: var(--accent-primary);
      box-shadow: 0 0 0 3px var(--accent-glow);
    }
    
    .action-btn {
      background: linear-gradient(135deg, var(--accent-primary) 0%, #ff8f5a 100%);
      border: none;
      border-radius: var(--radius-md);
      padding: 14px 30px;
      color: white;
      cursor: pointer;
      transition: all 0.2s ease;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.8rem;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 2px;
      width: 100%;
      position: relative;
      overflow: hidden;
    }
    
    .action-btn::before {
      content: '';
      position: absolute;
      top: 0;
      left: -100%;
      width: 100%;
      height: 100%;
      background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent);
      transition: left 0.5s ease;
    }
    
    .action-btn:hover::before {
      left: 100%;
    }
    
    .action-btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 8px 30px var(--accent-glow);
    }
    
    .action-btn:active {
      transform: translateY(0);
    }
    
    .footer {
      text-align: center;
      margin-top: 40px;
      padding: 20px;
    }
    
    .footer-text {
      font-family: 'JetBrains Mono', monospace;
      color: var(--text-muted);
      font-size: 0.65rem;
      letter-spacing: 3px;
      text-transform: uppercase;
    }
    
    .last-update {
      font-family: 'JetBrains Mono', monospace;
      color: var(--text-muted);
      font-size: 0.65rem;
      text-align: center;
      margin-top: 16px;
      letter-spacing: 1px;
    }
    
    @media (max-width: 768px) {
      .main-layout {
        grid-template-columns: 1fr;
      }
      .sensor-panel {
        position: static;
      }
      .sensor-grid {
        flex-direction: row;
        flex-wrap: wrap;
      }
      .sensor-card {
        flex: 1;
        min-width: 120px;
      }
      .controls-grid {
        grid-template-columns: 1fr;
        gap: 30px;
      }
      .position-group {
        grid-template-columns: 1fr;
      }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Paint<span>Machine</span></h1>
      <p>Control Interface</p>
    </div>
    
    <div class="main-layout">
      <div class="sensor-panel">
        <div class="panel-label">Sensors</div>
    <div class="sensor-grid" id="sensorGrid">
          <!-- Sensors populated by JS -->
        </div>
    </div>
    
      <div class="content-area">
        <div class="card">
          <div class="card-header">
            <div class="card-icon">&oplus;</div>
            <span class="card-title">Manual Controls</span>
          </div>
          <div class="card-body">
            <div class="controls-grid">
              <div class="control-section">
                <div class="control-label">Gantry X/Y</div>
                <div class="position-display">
                  <div class="pos-item">
                    <span class="pos-label">X</span>
                    <span class="pos-value" id="posX">0.00"</span>
          </div>
                  <div class="pos-item">
                    <span class="pos-label">Y</span>
                    <span class="pos-value" id="posY">0.00"</span>
                  </div>
                </div>
                <div class="arrow-controls">
                  <button class="arrow-btn up" onmousedown="moveY(-1)" onmouseup="stopMove()" ontouchstart="moveY(-1)" ontouchend="stopMove()">&uarr;</button>
                  <button class="arrow-btn left" onmousedown="moveX(-1)" onmouseup="stopMove()" ontouchstart="moveX(-1)" ontouchend="stopMove()">&larr;</button>
                  <button class="arrow-btn right" onmousedown="moveX(1)" onmouseup="stopMove()" ontouchstart="moveX(1)" ontouchend="stopMove()">&rarr;</button>
                  <button class="arrow-btn down" onmousedown="moveY(1)" onmouseup="stopMove()" ontouchstart="moveY(1)" ontouchend="stopMove()">&darr;</button>
                </div>
                <div class="distance-selector">
                  <button class="distance-btn" id="btn01in" onclick="setDistance(0.1)">.1"</button>
            <button class="distance-btn active" id="btn1in" onclick="setDistance(1)">1"</button>
            <button class="distance-btn" id="btn3in" onclick="setDistance(3)">3"</button>
          </div>
        </div>
              <div class="control-section">
                <div class="control-label">Fork Motor</div>
                <div class="position-display">
                  <div class="pos-item">
                    <span class="pos-label">Z</span>
                    <span class="pos-value" id="posFork">0.00"</span>
          </div>
          </div>
                <div class="arrow-controls" style="grid-template-columns: 48px; grid-template-rows: repeat(2, 48px);">
                  <button class="arrow-btn up" style="grid-column: 1; grid-row: 1;" onmousedown="moveFork(-1)" onmouseup="stopMove()" ontouchstart="moveFork(-1)" ontouchend="stopMove()">&uarr;</button>
                  <button class="arrow-btn down" style="grid-column: 1; grid-row: 2;" onmousedown="moveFork(1)" onmouseup="stopMove()" ontouchstart="moveFork(1)" ontouchend="stopMove()">&darr;</button>
                </div>
                <div class="distance-selector">
                  <button class="distance-btn" id="btnFork01in" onclick="setForkDistance(0.1)">.1"</button>
            <button class="distance-btn active" id="btnFork1in" onclick="setForkDistance(1)">1"</button>
            <button class="distance-btn" id="btnFork3in" onclick="setForkDistance(3)">3"</button>
          </div>
        </div>
              <div class="control-section">
                <div class="control-label">Storage Motor</div>
                <div class="arrow-controls" style="grid-template-columns: 48px; grid-template-rows: repeat(2, 48px);">
                  <button class="arrow-btn up" style="grid-column: 1; grid-row: 1;" onclick="moveStorage(-1)">&uarr;</button>
                  <button class="arrow-btn down" style="grid-column: 1; grid-row: 2;" onclick="moveStorage(1)">&darr;</button>
                </div>
        </div>
      </div>
      <div class="home-buttons">
        <button class="home-btn" onclick="homeAxis('x')">Home X</button>
        <button class="home-btn" onclick="homeAxis('y')">Home Y</button>
        <button class="home-btn" onclick="homeAxis('fork')">Home Fork Motor</button>
        <button class="home-btn all" onclick="homeAxis('all')">Home All</button>
            </div>
      </div>
    </div>
    
        <div class="card">
          <div class="card-header">
            <div class="card-icon">&raquo;</div>
            <span class="card-title">Test Sequence</span>
          </div>
          <div class="card-body">
      <div class="position-group">
        <div class="position-inputs">
          <h3>Position 1</h3>
          <div class="input-row">
            <label>X:</label>
            <input type="number" id="pos1X" step="0.1" value="0" placeholder="0.0">
          </div>
          <div class="input-row">
            <label>Y:</label>
            <input type="number" id="pos1Y" step="0.1" value="0" placeholder="0.0">
          </div>
          <div class="input-row">
            <label>Fork Motor:</label>
            <input type="number" id="pos1Fork" step="0.1" value="0" placeholder="0.0">
          </div>
        </div>
        <div class="position-inputs">
          <h3>Position 2</h3>
          <div class="input-row">
            <label>X:</label>
            <input type="number" id="pos2X" step="0.1" value="0" placeholder="0.0">
          </div>
          <div class="input-row">
            <label>Y:</label>
            <input type="number" id="pos2Y" step="0.1" value="0" placeholder="0.0">
          </div>
          <div class="input-row">
            <label>Fork Motor:</label>
            <input type="number" id="pos2Fork" step="0.1" value="0" placeholder="0.0">
          </div>
        </div>
      </div>
            <button class="action-btn" onclick="startTest()">Run Test</button>
          </div>
    </div>
    
        <div class="card">
      <div class="collapsible-header" onclick="toggleMotorSettings()">
            <div class="card-header-content">
              <div class="card-icon" style="background: linear-gradient(135deg, #666 0%, #888 100%);">*</div>
              <span class="card-title">Motor Settings</span>
            </div>
            <span class="chevron">&darr;</span>
      </div>
      <div class="collapsible-content" id="motorSettingsContent">
            <div class="card-body">
        <div class="position-group">
          <div class="position-inputs">
            <h3>X Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedX" step="100" min="100" max="10000" placeholder="2000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelX" step="100" min="100" max="10000" placeholder="5000">
            </div>
          </div>
          <div class="position-inputs">
            <h3>Y Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedY" step="100" min="100" max="10000" placeholder="2000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelY" step="100" min="100" max="10000" placeholder="5000">
            </div>
          </div>
        </div>
              <div class="position-group">
          <div class="position-inputs">
            <h3>Fork Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedFork" step="100" min="100" max="10000" placeholder="2000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelFork" step="100" min="100" max="10000" placeholder="5000">
            </div>
          </div>
                <div style="opacity: 0; pointer-events: none;"></div>
          </div>
              <button class="action-btn" style="background: linear-gradient(135deg, #666 0%, #888 100%);" onclick="saveMotorSettings()">Save Settings</button>
        </div>
      </div>
    </div>
    
    <div class="last-update" id="lastUpdate">Last update: --</div>
      </div>
    </div>
    
    <div class="footer">
      <div class="footer-text">Paint Machine Control System</div>
    </div>
  </div>
  
  <script>
    const sensors = [
      { id: 'xHome1', name: 'X Home 1' },
      { id: 'xHome2', name: 'X Home 2' },
      { id: 'yHome', name: 'Y Home' },
      { id: 'forkHome', name: 'Fork Home' },
      { id: 'testButton', name: 'Test Btn' }
    ];
    const STORAGE_MOTOR_STEPS_PER_CLICK = %STORAGE_MOTOR_STEPS_PER_CLICK_VALUE%;
    
    function createSensorCard(sensor, state) {
      const isActive = state === true || state === 1;
      return `
        <div class="sensor-card ${isActive ? 'active' : ''}" id="card-${sensor.id}">
          <div class="sensor-header">
            <span class="sensor-name">${sensor.name}</span>
              <div class="status-indicator"></div>
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
    
    // Update positions
    function updatePositions() {
      fetch('/api/positions')
        .then(response => response.json())
        .then(data => {
          document.getElementById('posX').textContent = data.posX.toFixed(1) + '"';
          document.getElementById('posY').textContent = data.posY.toFixed(1) + '"';
          document.getElementById('posFork').textContent = data.posFork.toFixed(1) + '"';
        })
        .catch(error => {
          console.error('Error fetching position data:', error);
        });
    }
    
    // Update positions immediately and then every 200ms
    updatePositions();
    setInterval(updatePositions, 200);
    
    // Load saved test positions on page load
    function loadTestPositions() {
      fetch('/api/test/positions')
        .then(response => response.json())
        .then(data => {
          document.getElementById('pos1X').value = data.pos1X || 0;
          document.getElementById('pos1Y').value = data.pos1Y || 0;
          document.getElementById('pos1Fork').value = data.pos1Fork || 0;
          document.getElementById('pos2X').value = data.pos2X || 0;
          document.getElementById('pos2Y').value = data.pos2Y || 0;
          document.getElementById('pos2Fork').value = data.pos2Fork || 0;
        })
        .catch(error => {
          console.error('Error loading test positions:', error);
        });
    }
    
    // Load saved motor settings on page load
    function loadMotorSettings() {
      fetch('/api/motor/settings')
        .then(response => response.json())
        .then(data => {
          document.getElementById('speedX').value = data.speedX || 2000;
          document.getElementById('accelX').value = data.accelX || 5000;
          document.getElementById('speedY').value = data.speedY || 2000;
          document.getElementById('accelY').value = data.accelY || 5000;
          document.getElementById('speedFork').value = data.speedFork || 2000;
          document.getElementById('accelFork').value = data.accelFork || 5000;
        })
        .catch(error => {
          console.error('Error loading motor settings:', error);
        });
    }
    
    // Save motor settings
    function saveMotorSettings() {
      const speedX = parseInt(document.getElementById('speedX').value) || 2000;
      const accelX = parseInt(document.getElementById('accelX').value) || 5000;
      const speedY = parseInt(document.getElementById('speedY').value) || 2000;
      const accelY = parseInt(document.getElementById('accelY').value) || 5000;
      const speedFork = parseInt(document.getElementById('speedFork').value) || 2000;
      const accelFork = parseInt(document.getElementById('accelFork').value) || 5000;
      
      const url = '/api/motor/settings?speedX=' + speedX + '&accelX=' + accelX +
                  '&speedY=' + speedY + '&accelY=' + accelY +
                  '&speedFork=' + speedFork + '&accelFork=' + accelFork;
      
      fetch(url)
        .then(response => response.text())
        .then(data => {
          console.log('Motor settings saved:', data);
        })
        .catch(error => {
          console.error('Error saving motor settings:', error);
        });
    }
    
    // Load positions and settings when page loads
    loadTestPositions();
    loadMotorSettings();
    
    // Movement control
    let moveDistance = 1; // Default 1 inch
    let forkDistance = 1; // Default 1 inch
    
    function setDistance(inches) {
      moveDistance = inches;
      document.getElementById('btn01in').classList.toggle('active', inches === 0.1);
      document.getElementById('btn1in').classList.toggle('active', inches === 1);
      document.getElementById('btn3in').classList.toggle('active', inches === 3);
    }
    
    function setForkDistance(inches) {
      forkDistance = inches;
      document.getElementById('btnFork01in').classList.toggle('active', inches === 0.1);
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
    
    function moveStorage(direction) {
      fetch('/api/move?axis=storage&steps=' + (direction * STORAGE_MOTOR_STEPS_PER_CLICK))
        .catch(error => console.error('Move error:', error));
    }
    
    function stopMove() {
      // Movement stops when button is released
      // The server handles the movement as a single command
    }
    
    function toggleMotorSettings() {
      const content = document.getElementById('motorSettingsContent');
      const header = document.querySelector('.collapsible-header');
      content.classList.toggle('expanded');
      header.classList.toggle('active');
    }
    
    function homeAxis(axis) {
      fetch('/api/home?axis=' + axis)
        .then(response => response.text())
        .then(data => {
          console.log('Home response:', data);
        })
        .catch(error => console.error('Home error:', error));
    }
    
    function startTest() {
      const pos1X = parseFloat(document.getElementById('pos1X').value) || 0;
      const pos1Y = parseFloat(document.getElementById('pos1Y').value) || 0;
      const pos1Fork = parseFloat(document.getElementById('pos1Fork').value) || 0;
      const pos2X = parseFloat(document.getElementById('pos2X').value) || 0;
      const pos2Y = parseFloat(document.getElementById('pos2Y').value) || 0;
      const pos2Fork = parseFloat(document.getElementById('pos2Fork').value) || 0;
      
      const url = '/api/test?pos1X=' + pos1X + '&pos1Y=' + pos1Y + '&pos1Fork=' + pos1Fork +
                  '&pos2X=' + pos2X + '&pos2Y=' + pos2Y + '&pos2Fork=' + pos2Fork;
      
      fetch(url)
        .then(response => response.text())
        .then(data => {
          console.log('Test started:', data);
        })
        .catch(error => console.error('Test error:', error));
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
    if (motorStorage == nullptr) {
        motorStorage = new StepperMotor(STORAGE_STEP_PIN, STORAGE_DIR_PIN, STEPS_PER_INCH, STORAGE_MOTOR_SPEED, STORAGE_MOTOR_ACCEL);
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

// Processor function to replace placeholder in HTML
String processor(const String& var) {
    if (var == "STORAGE_MOTOR_STEPS_PER_CLICK_VALUE") {
        return String(STORAGE_MOTOR_STEPS_PER_CLICK);
    }
    return "";
}

void initializeWebServer() {
    // Load saved test position values
    loadTestPositions();
    
    // Load saved motor settings
    loadMotorSettings();
    
    // Initialize sensor pins
    initializeSensors();
    
    // Initialize motors
    initializeMotors();
    
    // Apply saved motor settings to motors
    applyMotorSettings();

    // Route for root / web page (sensor dashboard)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", sensors_html, processor);
    });
    
    // API endpoint for sensor states
    server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorStatesJSON();
        request->send(200, "application/json", json);
    });
    
    // API endpoint for movement
    server.on("/api/move", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("axis")) {
            String axis = request->getParam("axis")->value();
            StepperMotor* motor = nullptr;
            const char* axisName = "";
            
            if (axis == "x") {
                if (request->hasParam("distance")) {
                    float distance = request->getParam("distance")->value().toFloat();
                    motor = motorX;
                    axisName = "X";
                    if (motor) {
                        motor->moveInches(distance);
                        request->send(200, "text/plain", "OK");
                        Serial.printf("Web Request: Move %s by %.2f inches\n", axisName, distance);
                    } else {
                        request->send(400, "text/plain", "Motor not initialized");
                    }
                } else {
                    request->send(400, "text/plain", "Missing distance parameter");
                }
            } else if (axis == "y") {
                if (request->hasParam("distance")) {
                    float distance = request->getParam("distance")->value().toFloat();
                    motor = motorY;
                    axisName = "Y";
                    if (motor) {
                        motor->moveInches(distance);
                        request->send(200, "text/plain", "OK");
                        Serial.printf("Web Request: Move %s by %.2f inches\n", axisName, distance);
                    } else {
                        request->send(400, "text/plain", "Motor not initialized");
                    }
                } else {
                    request->send(400, "text/plain", "Missing distance parameter");
                }
            } else if (axis == "fork") {
                if (request->hasParam("distance")) {
                    float distance = request->getParam("distance")->value().toFloat();
                    motor = motorFork;
                    axisName = "Fork Motor";
                    if (motor) {
                        motor->moveInches(distance);
                        request->send(200, "text/plain", "OK");
                        Serial.printf("Web Request: Move %s by %.2f inches\n", axisName, distance);
                    } else {
                        request->send(400, "text/plain", "Motor not initialized");
                    }
                } else {
                    request->send(400, "text/plain", "Missing distance parameter");
                }
            } else if (axis == "storage") {
                if (request->hasParam("steps")) {
                    long steps = request->getParam("steps")->value().toInt();
                    motor = motorStorage;
                    axisName = "Storage Motor";
                    if (motor) {
                        motor->moveSteps(steps);
                        request->send(200, "text/plain", "OK");
                        Serial.printf("Web Request: Move %s by %ld steps\n", axisName, steps);
                    } else {
                        request->send(400, "text/plain", "Motor not initialized");
                    }
                } else {
                    request->send(400, "text/plain", "Missing steps parameter");
                }
            } else {
                request->send(400, "text/plain", "Invalid axis");
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
                Serial.println("Web Request: Home Fork Motor axis");
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
    
    // API endpoint to get test position values
    server.on("/api/test/positions", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"pos1X\":" + String(testPos1X) + ",";
        json += "\"pos1Y\":" + String(testPos1Y) + ",";
        json += "\"pos1Fork\":" + String(testPos1Fork) + ",";
        json += "\"pos2X\":" + String(testPos2X) + ",";
        json += "\"pos2Y\":" + String(testPos2Y) + ",";
        json += "\"pos2Fork\":" + String(testPos2Fork);
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // API endpoint for test sequence
    server.on("/api/test", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("pos1X") && request->hasParam("pos1Y") && request->hasParam("pos1Fork") &&
            request->hasParam("pos2X") && request->hasParam("pos2Y") && request->hasParam("pos2Fork")) {
            
            // Get position values from request
            testPos1X = request->getParam("pos1X")->value().toFloat();
            testPos1Y = request->getParam("pos1Y")->value().toFloat();
            testPos1Fork = request->getParam("pos1Fork")->value().toFloat();
            testPos2X = request->getParam("pos2X")->value().toFloat();
            testPos2Y = request->getParam("pos2Y")->value().toFloat();
            testPos2Fork = request->getParam("pos2Fork")->value().toFloat();
            
            // Save values to persistent storage
            saveTestPositions();
            
            // Start test state (STATE_TEST = 2)
            extern void setMachineState(int state);
            setMachineState(2);
            
            request->send(200, "text/plain", "OK");
            Serial.println("Web Request: Start test sequence");
        } else {
            request->send(400, "text/plain", "Missing position parameters");
        }
    });
    
    // API endpoint for current positions
    server.on("/api/positions", HTTP_GET, [](AsyncWebServerRequest *request){
        float posX = 0.0;
        float posY = 0.0;
        float posFork = 0.0;
        
        if (motorX) {
            long steps = motorX->getCurrentPosition();
            posX = -motorX->stepsToInches(steps);  // Flip sign
        }
        if (motorY) {
            long steps = motorY->getCurrentPosition();
            posY = -motorY->stepsToInches(steps);  // Flip sign
        }
        if (motorFork) {
            long steps = motorFork->getCurrentPosition();
            posFork = -motorFork->stepsToInches(steps);  // Flip sign
        }
        
        String json = "{";
        json += "\"posX\":" + String(posX) + ",";
        json += "\"posY\":" + String(posY) + ",";
        json += "\"posFork\":" + String(posFork);
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // API endpoint for motor settings (GET to retrieve, GET with params to set)
    server.on("/api/motor/settings", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("speedX") && request->hasParam("accelX") &&
            request->hasParam("speedY") && request->hasParam("accelY") &&
            request->hasParam("speedFork") && request->hasParam("accelFork")) {
            
            // Get motor settings from request
            motorSpeedX = request->getParam("speedX")->value().toInt();
            motorAccelX = request->getParam("accelX")->value().toInt();
            motorSpeedY = request->getParam("speedY")->value().toInt();
            motorAccelY = request->getParam("accelY")->value().toInt();
            motorSpeedFork = request->getParam("speedFork")->value().toInt();
            motorAccelFork = request->getParam("accelFork")->value().toInt();
            
            // Apply settings to motors
            applyMotorSettings();
            
            // Save to persistent storage
            saveMotorSettings();
            
            request->send(200, "text/plain", "OK");
            Serial.println("Web Request: Motor settings updated");
        } else {
            // Return current settings if no parameters provided
            String json = "{";
            json += "\"speedX\":" + String(motorSpeedX) + ",";
            json += "\"accelX\":" + String(motorAccelX) + ",";
            json += "\"speedY\":" + String(motorSpeedY) + ",";
            json += "\"accelY\":" + String(motorAccelY) + ",";
            json += "\"speedFork\":" + String(motorSpeedFork) + ",";
            json += "\"accelFork\":" + String(motorAccelFork);
            json += "}";
            request->send(200, "application/json", json);
        }
    });

    server.begin();
    Serial.println("Web Server initialized");
    Serial.println("Sensor Dashboard available at /");
}


void updateWebServer() {
    // Web server handles requests asynchronously, no update needed
}
