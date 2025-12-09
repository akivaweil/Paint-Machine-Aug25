#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Bounce2.h>
#include "Web_Manager.h"
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/StorageMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"
#include "ServoControl.h"
#include "Paint_Motor_Controller.h"

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
StorageMotor* motorStorage = nullptr;

// Home switch instances
HomeSwitch* homeSwitchX = nullptr;
HomeSwitch* homeSwitchY = nullptr;
HomeSwitch* homeSwitchFork = nullptr;

// Storage Position Sensor (Bounce2 instance)
Bounce2::Button storagePositionSensor;

// Servo instance
ServoControl* servo = nullptr;
float currentServoAngle = SERVO_HOME_ANGLE;  // Track current servo position

// Suction and Paint Gun state
bool suctionState = false;
bool paintGunState = false;
bool pressurePotState = false;

// Test position values
float testPos1X = 0.0;
float testPos1Y = 0.0;
float testPos1Fork = 0.0;
float testPos2X = 0.0;
float testPos2Y = 0.0;
float testPos2Fork = 0.0;

// Position 1 height selection (1-8, where 8 = a8/lowest, 1 = a1/highest)
int selectedPosition1Height = 8;  // Default to a8 (lowest position)

// Column tracking (0-5, where 0 = column A, 5 = column F)
int currentColumn = 0;  // Current column position (set during homing)
int selectedColumn = 0;  // Selected column for test cycle (default: column A)

// Test All mode tracking
bool testAllMode = false;  // Flag to track if we're in "test all" mode
int currentTestAllHeight = 1;  // Track which height we're currently testing (1-8)
int testAllColumnCount = 1;  // Number of columns to test (1-6)
int testAllStartColumn = 0;  // Starting column position (from physical position)
int testAllCurrentColumnIndex = 0;  // Current column index in the test sequence (0 to testAllColumnCount-1)

// Square sensing toggle (enables/disables wood_present_sensor check during test cycle)
bool squareSensingEnabled = true;  // Default to enabled

// Test mode toggle (disables paint gun and pressure pot during paint cycle)
bool testModeEnabled = false;  // Default to disabled

// Cycle control state
bool cyclePaused = false;  // Tracks if cycle is paused
bool cycleCancelled = false;  // Tracks if cycle should be cancelled

// Motor speed and acceleration settings
long motorSpeedX = X_MAX_SPEED;
long motorSpeedY = Y_MAX_SPEED;
long motorSpeedFork = FORK_MAX_SPEED;
long motorAccelX = X_MAX_ACCEL;
long motorAccelY = Y_MAX_ACCEL;
long motorAccelFork = FORK_MAX_ACCEL;
long motorSpeedPaintRotation = PAINT_ROTATION_MOTOR_SPEED;
long motorAccelPaintRotation = PAINT_ROTATION_MOTOR_ACCEL;
long motorSpeedStorage = STORAGE_MOTOR_SPEED;
long motorAccelStorage = STORAGE_MOTOR_ACCEL;
long storageMotorStepsPerClick = STORAGE_MOTOR_STEPS_PER_CLICK;
long storageMotorTrimDistance = STORAGE_MOTOR_HOMING_TRIM;
long paintRotationMotorStepsPerClick = PAINT_ROTATION_MOTOR_STEPS_PER_CLICK;
long paintRotationMotorStepsPerRevOutput = PAINT_ROTATION_MOTOR_STEPS_PER_REV_OUTPUT;
float servoSpeed = 30.0;  // Servo speed in degrees per second (default 30 = 50% of typical 60)

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
    if (motorStorage) {
        motorStorage->setSpeed(motorSpeedStorage);
        motorStorage->setAcceleration(motorAccelStorage);
    }
    // Apply paint rotation motor settings
    setStepperSpeed(motorSpeedPaintRotation);
    setStepperAcceleration(motorAccelPaintRotation);
}

void initializeWebServer() {
    // Load saved test position values
    loadTestPositions();
    
    // Load saved motor settings
    loadMotorSettings();
    
    // Load saved square sensing state
    loadSquareSensingState();
    
    // Load saved test mode state
    loadTestModeState();
    
    // Initialize sensor pins
    initializeSensors();
    
    // Initialize motors
    initializeMotors();
    
    // Apply saved motor settings to motors
    applyMotorSettings();

    // Setup all API routes
    setupAPIRoutes();

    server.begin();
    Serial.println("Web Server initialized");
    Serial.println("Sensor Dashboard available at /");
}

void updateWebServer() {
    // Update Bounce2 sensors
    storagePositionSensor.update();
    
    // Run storage motor (AccelStepper needs run() called regularly)
    if (motorStorage) {
        motorStorage->run();
    }
}
