#include <Arduino.h>

//* ************************************************************************
//* ************************ HTML CONTENT **********************************
//* ************************************************************************

// Sensor Dashboard HTML
const char sensors_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta charset="UTF-8">
  <title>Paint Machine</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><text y='.9em' font-size='90'>🎨</text></svg>">
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
    
    .toggle-switch {
      position: relative;
      display: inline-block;
      width: 50px;
      height: 26px;
    }
    
    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }
    
    .toggle-slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: var(--bg-elevated);
      border: 1px solid var(--border-subtle);
      transition: 0.3s;
      border-radius: 26px;
    }
    
    .toggle-slider:before {
      position: absolute;
      content: "";
      height: 18px;
      width: 18px;
      left: 3px;
      bottom: 3px;
      background-color: var(--text-secondary);
      transition: 0.3s;
      border-radius: 50%;
    }
    
    .toggle-switch input:checked + .toggle-slider {
      background-color: var(--accent-secondary);
      border-color: var(--accent-secondary);
    }
    
    .toggle-switch input:checked + .toggle-slider:before {
      transform: translateX(24px);
      background-color: var(--bg-deep);
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
    
    .input-row input,
    .input-row select {
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
    
    .input-row input:focus,
    .input-row select:focus {
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
    
    .button-container {
      display: flex;
      gap: 10px;
      width: 100%;
    }
    
    .action-btn-secondary {
      background: linear-gradient(135deg, var(--accent-secondary) 0%, #00f5d4 100%);
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
      position: relative;
      overflow: hidden;
    }
    
    .action-btn-secondary::before {
      content: '';
      position: absolute;
      top: 0;
      left: -100%;
      width: 100%;
      height: 100%;
      background: linear-gradient(90deg, transparent, rgba(255,255,255,0.2), transparent);
      transition: left 0.5s ease;
    }
    
    .action-btn-secondary:hover::before {
      left: 100%;
    }
    
    .action-btn-secondary:hover {
      transform: translateY(-2px);
      box-shadow: 0 8px 30px var(--accent-green-glow);
    }
    
    .action-btn-secondary:active {
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
            <div class="card-icon">↕</div>
            <span class="card-title">Run Test Cycle</span>
          </div>
          <div class="card-body">
            <div class="panel-label" style="margin-bottom: 16px;">Height (a1-a8)</div>
            <div style="display: flex; gap: 20px; align-items: center; margin-bottom: 20px;">
              <div class="distance-selector" style="flex: 1;">
                <button class="distance-btn" id="heightA1" onclick="setHeight(1)">a1</button>
                <button class="distance-btn" id="heightA2" onclick="setHeight(2)">a2</button>
                <button class="distance-btn" id="heightA3" onclick="setHeight(3)">a3</button>
                <button class="distance-btn" id="heightA4" onclick="setHeight(4)">a4</button>
                <button class="distance-btn" id="heightA5" onclick="setHeight(5)">a5</button>
                <button class="distance-btn" id="heightA6" onclick="setHeight(6)">a6</button>
                <button class="distance-btn" id="heightA7" onclick="setHeight(7)">a7</button>
                <button class="distance-btn active" id="heightA8" onclick="setHeight(8)">a8</button>
              </div>
              <div style="display: flex; align-items: center; gap: 10px;">
                <span style="font-family: 'JetBrains Mono', monospace; font-size: 0.7rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 1px;">Square Sensing:</span>
                <label class="toggle-switch">
                  <input type="checkbox" id="squareSensingToggle" onchange="toggleSquareSensing()">
                  <span class="toggle-slider"></span>
                </label>
              </div>
            </div>
            <div class="button-container">
              <button class="action-btn" onclick="startTest()" style="width: 75%;">Run Test</button>
              <button class="action-btn-secondary" onclick="startTestAll()" style="width: 25%;">Test All</button>
            </div>
          </div>
    </div>
    
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
                  <button class="arrow-btn up" style="grid-column: 1; grid-row: 1;" onclick="moveStorage(-1)">↺</button>
                  <button class="arrow-btn down" style="grid-column: 1; grid-row: 2;" onclick="moveStorage(1)">↻</button>
          </div>
        </div>
              <div class="control-section">
                <div class="control-label">Paint Rotation Motor</div>
                <div class="arrow-controls" style="grid-template-columns: 48px; grid-template-rows: repeat(2, 48px);">
                  <button class="arrow-btn up" style="grid-column: 1; grid-row: 1;" onclick="movePaintRotation(-1)">↺</button>
                  <button class="arrow-btn down" style="grid-column: 1; grid-row: 2;" onclick="movePaintRotation(1)">↻</button>
          </div>
        </div>
              <div class="control-section">
                <div class="control-label">Servo (0-270&deg;)</div>
                <div style="margin-top: 16px; width: 125%;">
                  <input type="range" id="servoSlider" min="0" max="270" value="SERVO_HOME_ANGLE_VALUE" step="5" style="width: 100%; height: 8px; background: var(--bg-elevated); border-radius: 4px; outline: none; -webkit-appearance: none;" oninput="setServoAngle(this.value)">
                  <div style="display: flex; justify-content: space-between; margin-top: 8px; font-family: 'JetBrains Mono', monospace; font-size: 0.7rem; color: var(--text-muted);">
                    <span>0&deg;</span>
                    <span id="servoAngle" style="color: var(--accent-primary); font-weight: 600;">SERVO_HOME_ANGLE_VALUE&deg;</span>
                    <span>270&deg;</span>
                  </div>
                </div>
        </div>
      </div>
      <div style="margin-top: 24px; padding-top: 24px; border-top: 1px solid var(--border-subtle);">
        <div class="panel-label" style="margin-bottom: 16px;">Actuators</div>
        <div style="display: flex; gap: 40px; justify-content: center; align-items: center;">
          <div class="control-section" style="align-items: center;">
            <div class="control-label">Suction</div>
            <button class="distance-btn" id="suctionBtn" onclick="toggleSuction()" style="margin-top: 16px; width: 100px;">OFF</button>
          </div>
          <div class="control-section" style="align-items: center;">
            <div class="control-label">Paint Gun</div>
            <button class="distance-btn" id="paintGunBtn" onclick="togglePaintGun()" style="margin-top: 16px; width: 100px;">OFF</button>
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
          <h3>Pos1 + Pos4</h3>
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
          <h3>Pos2 + Pos3</h3>
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
          </div>
    </div>
    
        <div class="card">
      <div class="collapsible-header active" onclick="toggleMotorSettings()">
            <div class="card-header-content">
              <div class="card-icon" style="background: linear-gradient(135deg, #666 0%, #888 100%);">*</div>
              <span class="card-title">Motor Settings</span>
            </div>
            <span class="chevron">&darr;</span>
      </div>
      <div class="collapsible-content expanded" id="motorSettingsContent">
            <div class="card-body">
        <div class="position-group">
          <div class="position-inputs">
            <h3>X Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedX" step="100" min="100" max="30000" placeholder="2000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelX" step="100" min="100" max="30000" placeholder="5000">
            </div>
          </div>
          <div class="position-inputs">
            <h3>Y Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedY" step="100" min="100" max="30000" placeholder="2000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelY" step="100" min="100" max="30000" placeholder="5000">
            </div>
          </div>
        </div>
              <div class="position-group">
          <div class="position-inputs">
            <h3>Fork Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedFork" step="100" min="100" max="30000" placeholder="2000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelFork" step="100" min="100" max="30000" placeholder="5000">
            </div>
          </div>
          <div class="position-inputs">
            <h3>Storage Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedStorage" step="100" min="100" max="30000" placeholder="10000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelStorage" step="100" min="100" max="30000" placeholder="10000">
            </div>
            <div class="input-row">
              <label>Steps/Click:</label>
              <input type="number" id="storageSteps" step="100" min="100" max="1000000" placeholder="120000">
            </div>
          </div>
          <div class="position-inputs">
            <h3>Paint Rotation Motor</h3>
            <div class="input-row">
              <label>Speed:</label>
              <input type="number" id="speedPaintRotation" step="100" min="100" max="30000" placeholder="1000">
            </div>
            <div class="input-row">
              <label>Accel:</label>
              <input type="number" id="accelPaintRotation" step="100" min="100" max="30000" placeholder="1000">
            </div>
            <div class="input-row">
              <label>Steps/Click:</label>
              <input type="number" id="paintRotationSteps" step="100" min="100" max="1000000" placeholder="2000">
            </div>
            <div class="input-row">
              <label>Steps/Rev Output:</label>
              <input type="number" id="paintRotationRevOutput" step="100" min="100" max="1000000" placeholder="38400">
            </div>
          </div>
          <div class="position-inputs">
            <h3>Servo</h3>
            <div class="input-row">
              <label>Speed (deg/sec):</label>
              <input type="number" id="servoSpeed" step="1" min="1" max="120" placeholder="30">
            </div>
          </div>
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
      { id: 'testButton', name: 'Test Btn' },
      { id: 'storagePosition', name: 'Storage Position' },
      { id: 'squarePresent', name: 'Square Present' }
    ];
    let STORAGE_MOTOR_STEPS_PER_CLICK = STORAGE_MOTOR_STEPS_PER_CLICK_VALUE;
    let PAINT_ROTATION_MOTOR_STEPS_PER_CLICK = PAINT_ROTATION_MOTOR_STEPS_PER_CLICK_VALUE;
    let currentSpeedPaintRotation = 1000;
    let currentAccelPaintRotation = 1000;
    let currentPaintRotationSteps = PAINT_ROTATION_MOTOR_STEPS_PER_CLICK_VALUE;
    
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
    
    // Update immediately and then every 100ms
    updateSensors();
    setInterval(updateSensors, 100);
    
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
    
    // Connection monitoring for ESP reset detection
    let consecutiveFailures = 0;
    let espOffline = false;
    const MAX_FAILURES = 5; // Consider ESP offline after 5 failed requests (1 second)
    
    function checkConnection() {
      fetch('/api/sensors')
        .then(response => {
          if (response.ok) {
            consecutiveFailures = 0;
            // If ESP was offline and now it's back, refresh the page
            if (espOffline) {
              espOffline = false;
              console.log('ESP back online - refreshing page');
              window.location.reload();
            }
          } else {
            consecutiveFailures++;
            if (consecutiveFailures >= MAX_FAILURES && !espOffline) {
              espOffline = true;
              console.log('ESP appears to be offline');
            }
          }
        })
        .catch(error => {
          consecutiveFailures++;
          if (consecutiveFailures >= MAX_FAILURES && !espOffline) {
            espOffline = true;
            console.log('ESP appears to be offline');
          }
        });
    }
    
    // Check connection every 200ms
    setInterval(checkConnection, 200);
    
    // Load saved test positions on page load
    function loadTestPositions() {
      fetch('/api/test/positions')
        .then(response => response.json())
        .then(data => {
          document.getElementById('pos1X').value = data.pos1X || 0;
          document.getElementById('pos1Y').value = data.pos1Y || 0;
          document.getElementById('pos1Fork').value = data.pos1Fork || 0;
          const savedHeight = data.pos1Height || 8;
          setHeight(savedHeight);
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
          if (data.speedPaintRotation !== undefined) {
            document.getElementById('speedPaintRotation').value = data.speedPaintRotation;
            currentSpeedPaintRotation = data.speedPaintRotation;
          }
          if (data.accelPaintRotation !== undefined) {
            document.getElementById('accelPaintRotation').value = data.accelPaintRotation;
            currentAccelPaintRotation = data.accelPaintRotation;
          }
          if (data.speedStorage !== undefined) {
            document.getElementById('speedStorage').value = data.speedStorage;
          }
          if (data.accelStorage !== undefined) {
            document.getElementById('accelStorage').value = data.accelStorage;
          }
          if (data.storageSteps !== undefined) {
            document.getElementById('storageSteps').value = data.storageSteps;
            STORAGE_MOTOR_STEPS_PER_CLICK = data.storageSteps;
          }
          if (data.paintRotationSteps !== undefined) {
            document.getElementById('paintRotationSteps').value = data.paintRotationSteps;
            PAINT_ROTATION_MOTOR_STEPS_PER_CLICK = data.paintRotationSteps;
            currentPaintRotationSteps = data.paintRotationSteps;
          }
          if (data.paintRotationRevOutput !== undefined) {
            document.getElementById('paintRotationRevOutput').value = data.paintRotationRevOutput;
          }
          if (data.servoSpeed !== undefined) {
            document.getElementById('servoSpeed').value = data.servoSpeed;
          }
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
      const speedPaintRotationInput = document.getElementById('speedPaintRotation').value;
      const accelPaintRotationInput = document.getElementById('accelPaintRotation').value;
      const paintRotationStepsInput = document.getElementById('paintRotationSteps').value;
      const paintRotationRevOutputInput = document.getElementById('paintRotationRevOutput').value;
      const speedPaintRotation = speedPaintRotationInput ? parseInt(speedPaintRotationInput) : currentSpeedPaintRotation;
      const accelPaintRotation = accelPaintRotationInput ? parseInt(accelPaintRotationInput) : currentAccelPaintRotation;
      const speedStorage = parseInt(document.getElementById('speedStorage').value) || 10000;
      const accelStorage = parseInt(document.getElementById('accelStorage').value) || 10000;
      const storageSteps = parseInt(document.getElementById('storageSteps').value) || STORAGE_MOTOR_STEPS_PER_CLICK_VALUE;
      const paintRotationSteps = paintRotationStepsInput ? parseInt(paintRotationStepsInput) : currentPaintRotationSteps;
      const paintRotationRevOutput = paintRotationRevOutputInput ? parseInt(paintRotationRevOutputInput) : 38400;
      const servoSpeed = parseFloat(document.getElementById('servoSpeed').value) || 30;
      
      const url = '/api/motor/settings?speedX=' + speedX + '&accelX=' + accelX +
                  '&speedY=' + speedY + '&accelY=' + accelY +
                  '&speedFork=' + speedFork + '&accelFork=' + accelFork +
                  '&speedPaintRotation=' + speedPaintRotation + '&accelPaintRotation=' + accelPaintRotation +
                  '&speedStorage=' + speedStorage + '&accelStorage=' + accelStorage +
                  '&storageSteps=' + storageSteps + '&paintRotationSteps=' + paintRotationSteps + 
                  '&paintRotationRevOutput=' + paintRotationRevOutput + '&servoSpeed=' + servoSpeed;
      
      fetch(url)
        .then(response => response.text())
        .then(data => {
          console.log('Motor settings saved:', data);
          STORAGE_MOTOR_STEPS_PER_CLICK = storageSteps;
          PAINT_ROTATION_MOTOR_STEPS_PER_CLICK = paintRotationSteps;
          currentSpeedPaintRotation = speedPaintRotation;
          currentAccelPaintRotation = accelPaintRotation;
          currentPaintRotationSteps = paintRotationSteps;
        })
        .catch(error => {
          console.error('Error saving motor settings:', error);
        });
    }
    
    // Load positions and settings when page loads
    loadTestPositions();
    loadMotorSettings();
    loadDeviceStates();
    loadSquareSensingState();
    
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
    
    let selectedHeight = 8; // Default to a8 (lowest position)
    
    function setHeight(level) {
      selectedHeight = level;
      for (let i = 1; i <= 8; i++) {
        document.getElementById('heightA' + i).classList.toggle('active', i === level);
      }
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
    
    function movePaintRotation(direction) {
      fetch('/api/move?axis=paintRotation&steps=' + (direction * PAINT_ROTATION_MOTOR_STEPS_PER_CLICK))
        .catch(error => console.error('Move error:', error));
    }
    
    function setServoAngle(angle) {
      // Snap to nearest 5 degrees
      var snappedAngle = Math.round(angle / 5) * 5;
      document.getElementById('servoAngle').textContent = snappedAngle + '°';
      document.getElementById('servoSlider').value = snappedAngle;
      fetch('/api/servo?angle=' + snappedAngle)
        .catch(error => console.error('Servo error:', error));
    }
    
    function toggleSuction() {
      const btn = document.getElementById('suctionBtn');
      const isOn = btn.textContent === 'ON';
      fetch('/api/suction?state=' + (isOn ? 'off' : 'on'))
        .then(response => response.json())
        .then(data => {
          btn.textContent = data.state === 'on' ? 'ON' : 'OFF';
          btn.classList.toggle('active', data.state === 'on');
        })
        .catch(error => console.error('Suction error:', error));
    }
    
    function togglePaintGun() {
      const btn = document.getElementById('paintGunBtn');
      const isOn = btn.textContent === 'ON';
      fetch('/api/paintgun?state=' + (isOn ? 'off' : 'on'))
        .then(response => response.json())
        .then(data => {
          btn.textContent = data.state === 'on' ? 'ON' : 'OFF';
          btn.classList.toggle('active', data.state === 'on');
        })
        .catch(error => console.error('Paint gun error:', error));
    }
    
    // Load suction and paint gun states on page load
    function loadDeviceStates() {
      fetch('/api/devices/states')
        .then(response => response.json())
        .then(data => {
          const suctionBtn = document.getElementById('suctionBtn');
          const paintGunBtn = document.getElementById('paintGunBtn');
          if (suctionBtn) {
            suctionBtn.textContent = data.suction === 'on' ? 'ON' : 'OFF';
            suctionBtn.classList.toggle('active', data.suction === 'on');
          }
          if (paintGunBtn) {
            paintGunBtn.textContent = data.paintGun === 'on' ? 'ON' : 'OFF';
            paintGunBtn.classList.toggle('active', data.paintGun === 'on');
          }
        })
        .catch(error => console.error('Error loading device states:', error));
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
      const pos1Height = selectedHeight || 8;
      const pos2X = parseFloat(document.getElementById('pos2X').value) || 0;
      const pos2Y = parseFloat(document.getElementById('pos2Y').value) || 0;
      const pos2Fork = parseFloat(document.getElementById('pos2Fork').value) || 0;
      
      const url = '/api/test?pos1X=' + pos1X + '&pos1Y=' + pos1Y + '&pos1Fork=' + pos1Fork +
                  '&pos1Height=' + pos1Height +
                  '&pos2X=' + pos2X + '&pos2Y=' + pos2Y + '&pos2Fork=' + pos2Fork;
      
      fetch(url)
        .then(response => response.text())
        .then(data => {
          console.log('Test started:', data);
        })
        .catch(error => console.error('Test error:', error));
    }
    
    function startTestAll() {
      const pos1X = parseFloat(document.getElementById('pos1X').value) || 0;
      const pos1Y = parseFloat(document.getElementById('pos1Y').value) || 0;
      const pos1Fork = parseFloat(document.getElementById('pos1Fork').value) || 0;
      const pos2X = parseFloat(document.getElementById('pos2X').value) || 0;
      const pos2Y = parseFloat(document.getElementById('pos2Y').value) || 0;
      const pos2Fork = parseFloat(document.getElementById('pos2Fork').value) || 0;
      
      const url = '/api/test/all?pos1X=' + pos1X + '&pos1Y=' + pos1Y + '&pos1Fork=' + pos1Fork +
                  '&pos2X=' + pos2X + '&pos2Y=' + pos2Y + '&pos2Fork=' + pos2Fork;
      
      fetch(url)
        .then(response => response.text())
        .then(data => {
          console.log('Test All started:', data);
        })
        .catch(error => console.error('Test All error:', error));
    }
    
    function toggleSquareSensing() {
      const toggle = document.getElementById('squareSensingToggle');
      const enabled = toggle.checked;
      fetch('/api/squareSensing?enabled=' + (enabled ? '1' : '0'))
        .then(response => response.json())
        .then(data => {
          console.log('Square sensing:', data.enabled ? 'ON' : 'OFF');
        })
        .catch(error => console.error('Square sensing toggle error:', error));
    }
    
    // Load square sensing toggle state on page load
    function loadSquareSensingState() {
      fetch('/api/squareSensing')
        .then(response => response.json())
        .then(data => {
          const toggle = document.getElementById('squareSensingToggle');
          if (toggle) {
            toggle.checked = data.enabled;
          }
        })
        .catch(error => console.error('Error loading square sensing state:', error));
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
      } else if (e.key === 'h' || e.key === 'H') {
        e.preventDefault();
        homeAxis('all');
      }
    });
  </script>
</body>
</html>
)rawliteral";
