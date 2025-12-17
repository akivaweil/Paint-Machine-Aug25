#include "Web_Manager.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>

//* ************************************************************************
//* ************************ API ROUTES ***********************************
//* ************************************************************************

// Setup all API routes
void setupAPIRoutes() {
  // Route for root / web page (sensor dashboard)
  server.on("/", HTTP_GET, handleRoot);

  // API endpoint for sensor states
  server.on("/api/sensors", HTTP_GET, handleSensorStates);

  // API endpoint for storage motor clockwise movement with location finding
  // NOTE: Must be registered BEFORE /api/move to avoid prefix matching issues
  server.on("/api/storage/clockwise", HTTP_GET, handleStorageClockwise);

  // API endpoint for movement
  server.on("/api/move", HTTP_GET, handleMove);

  // API endpoint for homing
  server.on("/api/home", HTTP_GET, handleHome);

  // API endpoint to get/set test position values
  server.on("/api/test/positions", HTTP_GET, handleTestPositions);

  // API endpoint for test all sequence (runs all 8 heights a1-a8 for selected
  // column) NOTE: Must be registered BEFORE /api/test to avoid prefix matching
  // issues
  server.on("/api/test/all", HTTP_GET, handleTestAll);

  // API endpoint for single test sequence
  server.on("/api/test", HTTP_GET, handleTest);

  // API endpoint for current positions
  server.on("/api/positions", HTTP_GET, handlePositions);

  // API endpoint for motor settings (GET to retrieve, GET with params to set)
  server.on("/api/motor/settings", HTTP_GET, handleMotorSettings);

  // API endpoint for servo control
  server.on("/api/servo", HTTP_GET, handleServo);

  // API endpoint for suction control
  server.on("/api/suction", HTTP_GET, handleSuction);

  // API endpoint for paint gun control
  server.on("/api/paintgun", HTTP_GET, handlePaintGun);

  // API endpoint for pressure pot control
  server.on("/api/pressurepot", HTTP_GET, handlePressurePot);

  // API endpoint to get device states (reads actual pin states to ensure
  // accuracy)
  server.on("/api/devices/states", HTTP_GET, handleDeviceStates);

  // API endpoint for square sensing toggle
  server.on("/api/squareSensing", HTTP_GET, handleSquareSensing);

  // API endpoint for test mode toggle
  server.on("/api/testMode", HTTP_GET, handleTestMode);

  // API endpoint for skip painting toggle
  server.on("/api/skipPainting", HTTP_GET, handleSkipPainting);

  // API endpoint to get cycle state
  server.on("/api/cycle/state", HTTP_GET, handleCycleState);

  // API endpoint to pause/resume cycle
  server.on("/api/cycle/pause", HTTP_GET, handleCyclePause);

  // API endpoint to cancel cycle
  server.on("/api/cycle/cancel", HTTP_GET, handleCycleCancel);
}
