#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "Web_Manager.h"
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/StorageMotor.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/STATES/02_PICK_PLACE.h"
#include "Paint_Motor_Controller.h"

// Include HTML content (needed for sensors_html)
#include "Web_Manager/HTML_Content.cpp"

//* ************************************************************************
//* ************************ API ROUTES ***********************************
//* ************************************************************************

// Setup all API routes
void setupAPIRoutes() {
    // Route for root / web page (sensor dashboard)
    // Use PROGMEM response to avoid loading entire HTML into RAM
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        
        // Read from PROGMEM and write in chunks to avoid RAM overflow
        const char* html_P = sensors_html;
        size_t len = strlen_P(html_P);
        const size_t CHUNK_SIZE = 512;
        char buffer[CHUNK_SIZE + 1];
        
        for (size_t i = 0; i < len; i += CHUNK_SIZE) {
            size_t chunkLen = (i + CHUNK_SIZE < len) ? CHUNK_SIZE : (len - i);
            memcpy_P(buffer, html_P + i, chunkLen);
            buffer[chunkLen] = '\0';
            
            // Replace placeholders
            String chunk = String(buffer);
            chunk.replace("STORAGE_MOTOR_STEPS_PER_CLICK_VALUE", String(storageMotorStepsPerClick));
            chunk.replace("PAINT_ROTATION_MOTOR_STEPS_PER_CLICK_VALUE", String(paintRotationMotorStepsPerClick));
            chunk.replace("SERVO_HOME_ANGLE_VALUE", String(SERVO_HOME_ANGLE));
            
            response->print(chunk);
        }
        
        request->send(response);
    });
    
    // API endpoint for sensor states
    server.on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorStatesJSON();
        request->send(200, "application/json", json);
    });
    
    // API endpoint for storage motor clockwise movement with location finding
    // NOTE: Must be registered BEFORE /api/move to avoid prefix matching issues
    server.on("/api/storage/clockwise", HTTP_GET, [](AsyncWebServerRequest *request){
        moveStorageClockwise();
        request->send(200, "text/plain", "OK");
        Serial.println("Web Request: Move storage motor clockwise with location finding");
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
                    axisName = "Storage Motor";
                    if (motorStorage) {
                        // Stop any continuous movement and ensure motor is stopped
                        motorStorage->stopContinuous();
                        motorStorage->forceStop();
                        motorStorage->moveSteps(steps);
                        request->send(200, "text/plain", "OK");
                        Serial.printf("Web Request: Move %s by %ld steps\n", axisName, steps);
                    } else {
                        request->send(400, "text/plain", "Motor not initialized");
                    }
                } else {
                    request->send(400, "text/plain", "Missing steps parameter");
                }
            } else if (axis == "paintRotation") {
                if (request->hasParam("steps")) {
                    long steps = request->getParam("steps")->value().toInt();
                    axisName = "Paint Rotation Motor";
                    // Enable motor and wait for driver to stabilize
                    enablePaintRotationMotor();
                    delay(50);
                    // Turn on suction when painting motor starts rotating
                    digitalWrite(SUCTION_PIN, HIGH);
                    // Move the set amount (blocking)
                    moveStepper(steps);
                    // Block until motor finishes
                    while (isStepperRunning()) {
                        delay(10);
                    }
                    // Small delay before disabling to ensure movement is complete
                    delay(50);
                    // Turn off suction when motor stops
                    digitalWrite(SUCTION_PIN, LOW);
                    // Disable motor
                    disablePaintRotationMotor();
                    request->send(200, "text/plain", "OK");
                    Serial.printf("Web Request: Move %s by %ld steps\n", axisName, steps);
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
                homeAllAxes(false);  // false = normal homing (not boot), preserve column position
                request->send(200, "text/plain", "OK");
                Serial.println("Web Request: Home all axes");
            } else {
                request->send(400, "text/plain", "Invalid axis");
            }
        } else {
            request->send(400, "text/plain", "Missing axis parameter");
        }
    });
    
    // API endpoint to get/set test position values
    server.on("/api/test/positions", HTTP_GET, [](AsyncWebServerRequest *request){
        // Check if parameters are provided to set values
        if (request->hasParam("pos1X") || request->hasParam("pos1Y") || request->hasParam("pos1Fork") ||
            request->hasParam("pos2X") || request->hasParam("pos2Y") || request->hasParam("pos2Fork") ||
            request->hasParam("pos1Height") || request->hasParam("column")) {
            
            // Set position values if provided
            if (request->hasParam("pos1X")) {
                testPos1X = request->getParam("pos1X")->value().toFloat();
            }
            if (request->hasParam("pos1Y")) {
                testPos1Y = request->getParam("pos1Y")->value().toFloat();
            }
            if (request->hasParam("pos1Fork")) {
                testPos1Fork = request->getParam("pos1Fork")->value().toFloat();
            }
            if (request->hasParam("pos2X")) {
                testPos2X = request->getParam("pos2X")->value().toFloat();
            }
            if (request->hasParam("pos2Y")) {
                testPos2Y = request->getParam("pos2Y")->value().toFloat();
            }
            if (request->hasParam("pos2Fork")) {
                testPos2Fork = request->getParam("pos2Fork")->value().toFloat();
            }
            if (request->hasParam("pos1Height")) {
                int height = request->getParam("pos1Height")->value().toInt();
                if (height >= 1 && height <= 8) {
                    selectedPosition1Height = height;
                }
            }
            if (request->hasParam("column")) {
                int col = request->getParam("column")->value().toInt();
                if (col >= 0 && col <= 5) {
                    selectedColumn = col;
                }
            }
            
            // Save to persistent storage
            saveTestPositions();
            
            request->send(200, "text/plain", "OK");
            Serial.println("Web Request: Test positions updated");
        } else {
            // Return current values if no parameters provided
            String json = "{";
            json += "\"pos1X\":" + String(testPos1X) + ",";
            json += "\"pos1Y\":" + String(testPos1Y) + ",";
            json += "\"pos1Fork\":" + String(testPos1Fork) + ",";
            json += "\"pos1Height\":" + String(selectedPosition1Height) + ",";
            json += "\"column\":" + String(selectedColumn) + ",";
            json += "\"pos2X\":" + String(testPos2X) + ",";
            json += "\"pos2Y\":" + String(testPos2Y) + ",";
            json += "\"pos2Fork\":" + String(testPos2Fork);
            json += "}";
            request->send(200, "application/json", json);
        }
    });
    
    // API endpoint for test all sequence (runs all 8 heights a1-a8 for selected column)
    // NOTE: Must be registered BEFORE /api/test to avoid prefix matching issues
    server.on("/api/test/all", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("pos1X") && request->hasParam("pos1Y") && request->hasParam("pos1Fork") &&
            request->hasParam("pos2X") && request->hasParam("pos2Y") && request->hasParam("pos2Fork")) {
            
            // Set test all mode and start at height 1 (a1)
            testAllMode = true;
            currentTestAllHeight = 1;
            
            // Get position values from request
            testPos1X = request->getParam("pos1X")->value().toFloat();
            testPos1Y = request->getParam("pos1Y")->value().toFloat();
            testPos1Fork = request->getParam("pos1Fork")->value().toFloat();
            testPos2X = request->getParam("pos2X")->value().toFloat();
            testPos2Y = request->getParam("pos2Y")->value().toFloat();
            testPos2Fork = request->getParam("pos2Fork")->value().toFloat();
            
            // Get column count (1-6, default to 1)
            if (request->hasParam("columnCount")) {
                int count = request->getParam("columnCount")->value().toInt();
                if (count >= 1 && count <= 6) {
                    testAllColumnCount = count;
                } else {
                    testAllColumnCount = 1;  // Default to 1
                }
            } else {
                testAllColumnCount = 1;  // Default to 1
            }
            
            // Start from current physical column position
            extern int currentColumn;
            testAllStartColumn = currentColumn;
            testAllCurrentColumnIndex = 0;
            selectedColumn = currentColumn;  // Use physical position as starting column
            
            // Set position 1 height to 1 (a1, highest)
            selectedPosition1Height = 1;
            
            // Save values to persistent storage
            saveTestPositions();
            
            // Start pick and place state (STATE_PICK_PLACE = 2)
            extern void setMachineState(int state);
            setMachineState(2);
            
            request->send(200, "text/plain", "OK");
            Serial.printf("Web Request: Start test all sequence (a1-a8) for %d columns starting at column %c\n", 
                         testAllColumnCount, 'A' + selectedColumn);
        } else {
            request->send(400, "text/plain", "Missing position parameters");
        }
    });
    
    // API endpoint for single test sequence
    server.on("/api/test", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("pos1X") && request->hasParam("pos1Y") && request->hasParam("pos1Fork") &&
            request->hasParam("pos2X") && request->hasParam("pos2Y") && request->hasParam("pos2Fork")) {
            
            // Ensure test all mode is OFF for single test
            testAllMode = false;
            
            // Get position values from request
            testPos1X = request->getParam("pos1X")->value().toFloat();
            testPos1Y = request->getParam("pos1Y")->value().toFloat();
            testPos1Fork = request->getParam("pos1Fork")->value().toFloat();
            testPos2X = request->getParam("pos2X")->value().toFloat();
            testPos2Y = request->getParam("pos2Y")->value().toFloat();
            testPos2Fork = request->getParam("pos2Fork")->value().toFloat();
            
            // Get position 1 height selection (default to 8 if not provided)
            if (request->hasParam("pos1Height")) {
                selectedPosition1Height = request->getParam("pos1Height")->value().toInt();
                // Validate range (1-8)
                if (selectedPosition1Height < 1 || selectedPosition1Height > 8) {
                    selectedPosition1Height = 8;
                }
            }
            
            // Get column selection (a-f, convert to 0-5)
            if (request->hasParam("column")) {
                String columnStr = request->getParam("column")->value();
                if (columnStr.length() == 1) {
                    char colChar = columnStr.charAt(0);
                    if (colChar >= 'a' && colChar <= 'f') {
                        selectedColumn = colChar - 'a';
                    } else if (colChar >= 'A' && colChar <= 'F') {
                        selectedColumn = colChar - 'A';
                    } else {
                        selectedColumn = 0;  // Default to column A
                    }
                } else {
                    selectedColumn = 0;  // Default to column A
                }
            } else {
                selectedColumn = 0;  // Default to column A
            }
            
            // Save values to persistent storage
            saveTestPositions();
            
            // Start pick and place state (STATE_PICK_PLACE = 2)
            extern void setMachineState(int state);
            setMachineState(2);
            
            request->send(200, "text/plain", "OK");
            Serial.printf("Web Request: Start test sequence for column %c\n", 'A' + selectedColumn);
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
            request->hasParam("speedFork") && request->hasParam("accelFork") &&
            request->hasParam("speedPaintRotation") && request->hasParam("accelPaintRotation") &&
            request->hasParam("speedStorage") && request->hasParam("accelStorage")) {
            
            // Get motor settings from request
            motorSpeedX = request->getParam("speedX")->value().toInt();
            motorAccelX = request->getParam("accelX")->value().toInt();
            motorSpeedY = request->getParam("speedY")->value().toInt();
            motorAccelY = request->getParam("accelY")->value().toInt();
            motorSpeedFork = request->getParam("speedFork")->value().toInt();
            motorAccelFork = request->getParam("accelFork")->value().toInt();
            
            // Get paint rotation speed and accel (always sent from frontend)
            motorSpeedPaintRotation = request->getParam("speedPaintRotation")->value().toInt();
            motorAccelPaintRotation = request->getParam("accelPaintRotation")->value().toInt();
            
            // Get storage motor speed and accel (always sent from frontend)
            motorSpeedStorage = request->getParam("speedStorage")->value().toInt();
            motorAccelStorage = request->getParam("accelStorage")->value().toInt();
            
            // Get storage steps if provided
            if (request->hasParam("storageSteps")) {
                storageMotorStepsPerClick = request->getParam("storageSteps")->value().toInt();
            }
            
            // Get storage trim distance if provided
            if (request->hasParam("storageTrim")) {
                storageMotorTrimDistance = request->getParam("storageTrim")->value().toInt();
            }
            
            // Get paint rotation steps if provided
            if (request->hasParam("paintRotationSteps")) {
                paintRotationMotorStepsPerClick = request->getParam("paintRotationSteps")->value().toInt();
            }
            
            // Get paint rotation steps per rev output if provided
            if (request->hasParam("paintRotationRevOutput")) {
                paintRotationMotorStepsPerRevOutput = request->getParam("paintRotationRevOutput")->value().toInt();
            }
            
            // Get servo speed if provided
            if (request->hasParam("servoSpeed")) {
                servoSpeed = request->getParam("servoSpeed")->value().toFloat();
                if (servoSpeed < 1.0) servoSpeed = 1.0;
                if (servoSpeed > 120.0) servoSpeed = 120.0;
            }
            
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
            json += "\"accelFork\":" + String(motorAccelFork) + ",";
            json += "\"speedPaintRotation\":" + String(motorSpeedPaintRotation) + ",";
            json += "\"accelPaintRotation\":" + String(motorAccelPaintRotation) + ",";
            json += "\"speedStorage\":" + String(motorSpeedStorage) + ",";
            json += "\"accelStorage\":" + String(motorAccelStorage) + ",";
            json += "\"storageSteps\":" + String(storageMotorStepsPerClick) + ",";
            json += "\"storageTrim\":" + String(storageMotorTrimDistance) + ",";
            json += "\"paintRotationSteps\":" + String(paintRotationMotorStepsPerClick) + ",";
            json += "\"paintRotationRevOutput\":" + String(paintRotationMotorStepsPerRevOutput) + ",";
            json += "\"servoSpeed\":" + String(servoSpeed);
            json += "}";
            request->send(200, "application/json", json);
        }
    });
    
    // API endpoint for servo control
    server.on("/api/servo", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("angle")) {
            float angle = request->getParam("angle")->value().toFloat();
            setServoAngle(angle);
            request->send(200, "text/plain", "OK");
            Serial.printf("Web Request: Set servo to %.1f degrees\n", angle);
        } else {
            request->send(400, "text/plain", "Missing angle parameter");
        }
    });
    
    // API endpoint for suction control
    server.on("/api/suction", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("state")) {
            String state = request->getParam("state")->value();
            bool on = (state == "on");
            digitalWrite(SUCTION_PIN, on ? HIGH : LOW);
            suctionState = on;
            String json = "{\"state\":\"" + state + "\"}";
            request->send(200, "application/json", json);
            Serial.printf("Web Request: Set suction %s\n", on ? "ON" : "OFF");
        } else {
            request->send(400, "text/plain", "Missing state parameter");
        }
    });
    
    // API endpoint for paint gun control
    server.on("/api/paintgun", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("state")) {
            String state = request->getParam("state")->value();
            bool on = (state == "on");
            digitalWrite(PAINT_GUN_PIN, on ? HIGH : LOW);
            paintGunState = on;
            String json = "{\"state\":\"" + state + "\"}";
            request->send(200, "application/json", json);
            Serial.printf("Web Request: Set paint gun %s\n", on ? "ON" : "OFF");
        } else {
            request->send(400, "text/plain", "Missing state parameter");
        }
    });
    
    // API endpoint for pressure pot control
    server.on("/api/pressurepot", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("state")) {
            String state = request->getParam("state")->value();
            bool on = (state == "on");
            digitalWrite(PRESSURE_POT_PIN, on ? HIGH : LOW);
            pressurePotState = on;
            String json = "{\"state\":\"" + state + "\"}";
            request->send(200, "application/json", json);
            Serial.printf("Web Request: Set pressure pot %s\n", on ? "ON" : "OFF");
        } else {
            request->send(400, "text/plain", "Missing state parameter");
        }
    });
    
    // API endpoint to get device states (reads actual pin states to ensure accuracy)
    server.on("/api/devices/states", HTTP_GET, [](AsyncWebServerRequest *request){
        // Read actual pin states to ensure accuracy
        bool suctionOn = digitalRead(SUCTION_PIN) == HIGH;
        bool paintGunOn = digitalRead(PAINT_GUN_PIN) == HIGH;
        bool pressurePotOn = digitalRead(PRESSURE_POT_PIN) == HIGH;
        
        // Update state variables to keep them in sync
        suctionState = suctionOn;
        paintGunState = paintGunOn;
        pressurePotState = pressurePotOn;
        
        String json = "{";
        json += "\"suction\":\"" + String(suctionOn ? "on" : "off") + "\",";
        json += "\"paintGun\":\"" + String(paintGunOn ? "on" : "off") + "\",";
        json += "\"pressurePot\":\"" + String(pressurePotOn ? "on" : "off") + "\"";
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // API endpoint for square sensing toggle
    server.on("/api/squareSensing", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("enabled")) {
            int enabled = request->getParam("enabled")->value().toInt();
            squareSensingEnabled = (enabled == 1);
            saveSquareSensingState();
            String json = "{\"enabled\":" + String(squareSensingEnabled ? "true" : "false") + "}";
            request->send(200, "application/json", json);
            Serial.printf("Web Request: Square sensing %s\n", squareSensingEnabled ? "ENABLED" : "DISABLED");
        } else {
            // Return current state if no parameter provided
            String json = "{\"enabled\":" + String(squareSensingEnabled ? "true" : "false") + "}";
            request->send(200, "application/json", json);
        }
    });
    
    // API endpoint for test mode toggle
    server.on("/api/testMode", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("enabled")) {
            int enabled = request->getParam("enabled")->value().toInt();
            testModeEnabled = (enabled == 1);
            saveTestModeState();
            String json = "{\"enabled\":" + String(testModeEnabled ? "true" : "false") + "}";
            request->send(200, "application/json", json);
            Serial.printf("Web Request: Test mode %s\n", testModeEnabled ? "ENABLED" : "DISABLED");
        } else {
            // Return current state if no parameter provided
            String json = "{\"enabled\":" + String(testModeEnabled ? "true" : "false") + "}";
            request->send(200, "application/json", json);
        }
    });
    
    // API endpoint for skip painting toggle
    server.on("/api/skipPainting", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("enabled")) {
            int enabled = request->getParam("enabled")->value().toInt();
            skipPaintingEnabled = (enabled == 1);
            saveSkipPaintingState();
            String json = "{\"enabled\":" + String(skipPaintingEnabled ? "true" : "false") + "}";
            request->send(200, "application/json", json);
            Serial.printf("Web Request: Skip painting %s\n", skipPaintingEnabled ? "ENABLED" : "DISABLED");
        } else {
            // Return current state if no parameter provided
            String json = "{\"enabled\":" + String(skipPaintingEnabled ? "true" : "false") + "}";
            request->send(200, "application/json", json);
        }
    });
    
    // API endpoint to get cycle state
    server.on("/api/cycle/state", HTTP_GET, [](AsyncWebServerRequest *request){
        extern int getMachineState();
        int state = getMachineState();
        String stateStr = "IDLE";
        if (state == 0) stateStr = "HOMING";
        else if (state == 2) stateStr = "PICK_PLACE";
        else if (state == 3) stateStr = "PAINTING";
        
        String json = "{";
        json += "\"state\":\"" + stateStr + "\",";
        json += "\"paused\":" + String(cyclePaused ? "true" : "false");
        json += "}";
        request->send(200, "application/json", json);
    });
    
    // API endpoint to pause/resume cycle
    server.on("/api/cycle/pause", HTTP_GET, [](AsyncWebServerRequest *request){
        cyclePaused = !cyclePaused;
        String json = "{\"paused\":" + String(cyclePaused ? "true" : "false") + "}";
        request->send(200, "application/json", json);
        Serial.printf("Web Request: Cycle %s\n", cyclePaused ? "PAUSED" : "RESUMED");
    });
    
    // API endpoint to cancel cycle
    server.on("/api/cycle/cancel", HTTP_GET, [](AsyncWebServerRequest *request){
        cycleCancelled = true;
        cyclePaused = false;  // Clear pause flag when cancelling
        request->send(200, "text/plain", "OK");
        Serial.println("Web Request: Cycle CANCELLED");
    });
}

