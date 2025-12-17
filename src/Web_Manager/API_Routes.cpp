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
                        // Enable motor for movement (checks if homed)
                        motorStorage->enableMotor();
                        // Stop any continuous movement and ensure motor is stopped
                        motorStorage->stopContinuous();
                        motorStorage->forceStop();
                        motorStorage->moveSteps(steps);
                        // Wait for movement to complete
                        while (motorStorage->isMotorRunning()) {
                            motorStorage->run();
                            delay(1);
                        }
                        // Disable motor after movement completes
                        motorStorage->disableMotor();
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
    
    // API endpoint for painting configuration (GET to retrieve, GET with params to set)
    server.on("/api/painting/config", HTTP_GET, [](AsyncWebServerRequest *request){
        // Check if any parameters are provided to set values
        bool hasParams = request->hasParam("svHomeAng") || request->hasParam("svPaintAng") || 
                        request->hasParam("svUpdInt") || request->hasParam("svTgtThr") ||
                        request->hasParam("svFarThr") || request->hasParam("svMinMov") ||
                        request->hasParam("svMaxStp") || request->hasParam("step1XOff") ||
                        request->hasParam("step1YOff") || request->hasParam("step2Del") ||
                        request->hasParam("step3Fast") || request->hasParam("step4Del") ||
                        request->hasParam("step4Rot") || request->hasParam("step5Rot") ||
                        request->hasParam("step6Wait") || request->hasParam("step7Rot") ||
                        request->hasParam("step8Wait") || request->hasParam("step8Ang") ||
                        request->hasParam("step8Spd") || request->hasParam("step8Del") ||
                        request->hasParam("step9Rot") || request->hasParam("step10Wait") ||
                        request->hasParam("step10Ang") || request->hasParam("step10Spd") ||
                        request->hasParam("step10Del") || request->hasParam("step11Rot") ||
                        request->hasParam("step12Wait") || request->hasParam("step12Ang") ||
                        request->hasParam("step12Spd") || request->hasParam("step12Del") ||
                        request->hasParam("step13Rot") || request->hasParam("step14Wait") ||
                        request->hasParam("step14Ang") || request->hasParam("step14Spd") ||
                        request->hasParam("step14Del") || request->hasParam("step15Del") ||
                        request->hasParam("step15Rot") || request->hasParam("step15Ang") ||
                        request->hasParam("step16Ang") || request->hasParam("step18Del") ||
                        request->hasParam("step17Ang") || request->hasParam("step17Wait") ||
                        request->hasParam("step17Spd");
        
        if (hasParams) {
            // Set configuration values if provided
            if (request->hasParam("svHomeAng")) cfgServoHomeAngle = request->getParam("svHomeAng")->value().toFloat();
            if (request->hasParam("svPaintAng")) cfgServoPaintingAngle = request->getParam("svPaintAng")->value().toFloat();
            if (request->hasParam("svUpdInt")) cfgServoUpdateIntervalMs = request->getParam("svUpdInt")->value().toInt();
            if (request->hasParam("svTgtThr")) cfgServoTargetReachedThresholdDeg = request->getParam("svTgtThr")->value().toFloat();
            if (request->hasParam("svFarThr")) cfgServoFarFromTargetThresholdDeg = request->getParam("svFarThr")->value().toFloat();
            if (request->hasParam("svMinMov")) cfgServoMinMovementDeg = request->getParam("svMinMov")->value().toFloat();
            if (request->hasParam("svMaxStp")) cfgServoMaxStepSizeDeg = request->getParam("svMaxStp")->value().toFloat();
            if (request->hasParam("step1XOff")) cfgStep1WaitingPositionXOffsetInches = request->getParam("step1XOff")->value().toFloat();
            if (request->hasParam("step1YOff")) cfgStep1WaitingPositionYOffsetInches = request->getParam("step1YOff")->value().toFloat();
            if (request->hasParam("step2Del")) cfgStep2WaitingPositionDelayMs = request->getParam("step2Del")->value().toInt();
            if (request->hasParam("step3Fast")) cfgStep3ServoFastSpeed = request->getParam("step3Fast")->value().toFloat();
            if (request->hasParam("step4Del")) cfgStep4InitialRotationDelayMs = request->getParam("step4Del")->value().toInt();
            if (request->hasParam("step4Rot")) cfgStep4InitialRotationRev = request->getParam("step4Rot")->value().toFloat();
            if (request->hasParam("step5Rot")) cfgStep5LeftRotationRev = request->getParam("step5Rot")->value().toFloat();
            if (request->hasParam("step6Wait")) cfgStep6LeftSideWaitMs = request->getParam("step6Wait")->value().toInt();
            if (request->hasParam("step7Rot")) cfgStep7BackLeftRotationRev = request->getParam("step7Rot")->value().toFloat();
            if (request->hasParam("step8Wait")) cfgStep8BackLeftWaitMs = request->getParam("step8Wait")->value().toInt();
            if (request->hasParam("step8Ang")) cfgStep8ServoBackAngleDeg = request->getParam("step8Ang")->value().toFloat();
            if (request->hasParam("step8Spd")) cfgStep8BackLeftServoSpeed = request->getParam("step8Spd")->value().toFloat();
            if (request->hasParam("step8Del")) cfgStep8BackLeftPaintDelayMs = request->getParam("step8Del")->value().toInt();
            if (request->hasParam("step9Rot")) cfgStep9BackRotationRev = request->getParam("step9Rot")->value().toFloat();
            if (request->hasParam("step10Wait")) cfgStep10BackSideWaitMs = request->getParam("step10Wait")->value().toInt();
            if (request->hasParam("step10Ang")) cfgStep10ServoBackAngleDeg = request->getParam("step10Ang")->value().toFloat();
            if (request->hasParam("step10Spd")) cfgStep10BackServoSpeed = request->getParam("step10Spd")->value().toFloat();
            if (request->hasParam("step10Del")) cfgStep10BackPaintDelayMs = request->getParam("step10Del")->value().toInt();
            if (request->hasParam("step11Rot")) cfgStep11BackRightRotationRev = request->getParam("step11Rot")->value().toFloat();
            if (request->hasParam("step12Wait")) cfgStep12BackRightWaitMs = request->getParam("step12Wait")->value().toInt();
            if (request->hasParam("step12Ang")) cfgStep12ServoBackAngleDeg = request->getParam("step12Ang")->value().toFloat();
            if (request->hasParam("step12Spd")) cfgStep12BackRightServoSpeed = request->getParam("step12Spd")->value().toFloat();
            if (request->hasParam("step12Del")) cfgStep12BackRightPaintDelayMs = request->getParam("step12Del")->value().toInt();
            if (request->hasParam("step13Rot")) cfgStep13RightRotationRev = request->getParam("step13Rot")->value().toFloat();
            if (request->hasParam("step14Wait")) cfgStep14RightSideWaitMs = request->getParam("step14Wait")->value().toInt();
            if (request->hasParam("step14Ang")) cfgStep14ServoRightAngleDeg = request->getParam("step14Ang")->value().toFloat();
            if (request->hasParam("step14Spd")) cfgStep14RightServoSpeed = request->getParam("step14Spd")->value().toFloat();
            if (request->hasParam("step14Del")) cfgStep14RightPaintDelayMs = request->getParam("step14Del")->value().toInt();
            if (request->hasParam("step15Del")) cfgStep15PaintGunOffDelayMs = request->getParam("step15Del")->value().toInt();
            if (request->hasParam("step15Rot")) cfgStep15FinalRotationRev = request->getParam("step15Rot")->value().toFloat();
            if (request->hasParam("step15Ang")) cfgStep15FirstRevServoAngleDeg = request->getParam("step15Ang")->value().toFloat();
            if (request->hasParam("step16Ang")) cfgStep16SpinServoAngleDeg = request->getParam("step16Ang")->value().toFloat();
            if (request->hasParam("step18Del")) cfgStep18PaintGunOffDelayMs = request->getParam("step18Del")->value().toInt();
            if (request->hasParam("step17Ang")) cfgStep17InitialServoAngleDeg = request->getParam("step17Ang")->value().toFloat();
            if (request->hasParam("step17Wait")) cfgStep17InitialAngleWaitMs = request->getParam("step17Wait")->value().toInt();
            if (request->hasParam("step17Spd")) cfgStep17ServoSpeed = request->getParam("step17Spd")->value().toFloat();
            
            // Save to persistent storage
            savePaintingConfig();
            
            request->send(200, "text/plain", "OK");
            Serial.println("Web Request: Painting config updated");
        } else {
            // Return current configuration values as JSON
            String json = "{";
            json += "\"svHomeAng\":" + String(cfgServoHomeAngle) + ",";
            json += "\"svPaintAng\":" + String(cfgServoPaintingAngle) + ",";
            json += "\"svUpdInt\":" + String(cfgServoUpdateIntervalMs) + ",";
            json += "\"svTgtThr\":" + String(cfgServoTargetReachedThresholdDeg) + ",";
            json += "\"svFarThr\":" + String(cfgServoFarFromTargetThresholdDeg) + ",";
            json += "\"svMinMov\":" + String(cfgServoMinMovementDeg) + ",";
            json += "\"svMaxStp\":" + String(cfgServoMaxStepSizeDeg) + ",";
            json += "\"step1XOff\":" + String(cfgStep1WaitingPositionXOffsetInches) + ",";
            json += "\"step1YOff\":" + String(cfgStep1WaitingPositionYOffsetInches) + ",";
            json += "\"step2Del\":" + String(cfgStep2WaitingPositionDelayMs) + ",";
            json += "\"step3Fast\":" + String(cfgStep3ServoFastSpeed) + ",";
            json += "\"step4Del\":" + String(cfgStep4InitialRotationDelayMs) + ",";
            json += "\"step4Rot\":" + String(cfgStep4InitialRotationRev) + ",";
            json += "\"step5Rot\":" + String(cfgStep5LeftRotationRev) + ",";
            json += "\"step6Wait\":" + String(cfgStep6LeftSideWaitMs) + ",";
            json += "\"step7Rot\":" + String(cfgStep7BackLeftRotationRev) + ",";
            json += "\"step8Wait\":" + String(cfgStep8BackLeftWaitMs) + ",";
            json += "\"step8Ang\":" + String(cfgStep8ServoBackAngleDeg) + ",";
            json += "\"step8Spd\":" + String(cfgStep8BackLeftServoSpeed) + ",";
            json += "\"step8Del\":" + String(cfgStep8BackLeftPaintDelayMs) + ",";
            json += "\"step9Rot\":" + String(cfgStep9BackRotationRev) + ",";
            json += "\"step10Wait\":" + String(cfgStep10BackSideWaitMs) + ",";
            json += "\"step10Ang\":" + String(cfgStep10ServoBackAngleDeg) + ",";
            json += "\"step10Spd\":" + String(cfgStep10BackServoSpeed) + ",";
            json += "\"step10Del\":" + String(cfgStep10BackPaintDelayMs) + ",";
            json += "\"step11Rot\":" + String(cfgStep11BackRightRotationRev) + ",";
            json += "\"step12Wait\":" + String(cfgStep12BackRightWaitMs) + ",";
            json += "\"step12Ang\":" + String(cfgStep12ServoBackAngleDeg) + ",";
            json += "\"step12Spd\":" + String(cfgStep12BackRightServoSpeed) + ",";
            json += "\"step12Del\":" + String(cfgStep12BackRightPaintDelayMs) + ",";
            json += "\"step13Rot\":" + String(cfgStep13RightRotationRev) + ",";
            json += "\"step14Wait\":" + String(cfgStep14RightSideWaitMs) + ",";
            json += "\"step14Ang\":" + String(cfgStep14ServoRightAngleDeg) + ",";
            json += "\"step14Spd\":" + String(cfgStep14RightServoSpeed) + ",";
            json += "\"step14Del\":" + String(cfgStep14RightPaintDelayMs) + ",";
            json += "\"step15Del\":" + String(cfgStep15PaintGunOffDelayMs) + ",";
            json += "\"step15Rot\":" + String(cfgStep15FinalRotationRev) + ",";
            json += "\"step15Ang\":" + String(cfgStep15FirstRevServoAngleDeg) + ",";
            json += "\"step16Ang\":" + String(cfgStep16SpinServoAngleDeg) + ",";
            json += "\"step18Del\":" + String(cfgStep18PaintGunOffDelayMs) + ",";
            json += "\"step17Ang\":" + String(cfgStep17InitialServoAngleDeg) + ",";
            json += "\"step17Wait\":" + String(cfgStep17InitialAngleWaitMs) + ",";
            json += "\"step17Spd\":" + String(cfgStep17ServoSpeed);
            json += "}";
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

