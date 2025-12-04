#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Bounce2.h>
#include "Web_Manager.h"
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"
#include "ServoControl.h"

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
StepperMotor* motorPaintRotation = nullptr;

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

// Test position values
float testPos1X = 0.0;
float testPos1Y = 0.0;
float testPos1Fork = 0.0;
float testPos2X = 0.0;
float testPos2Y = 0.0;
float testPos2Fork = 0.0;

// Position 1 height selection (1-8, where 8 = a8/lowest, 1 = a1/highest)
int selectedPosition1Height = 8;  // Default to a8 (lowest position)

// Test All mode tracking
bool testAllMode = false;  // Flag to track if we're in "test all" mode
int currentTestAllHeight = 1;  // Track which height we're currently testing (1-8)

// Motor speed and acceleration settings
long motorSpeedX = X_MAX_SPEED;
long motorSpeedY = Y_MAX_SPEED;
long motorSpeedFork = FORK_MAX_SPEED;
long motorAccelX = X_MAX_ACCEL;
long motorAccelY = Y_MAX_ACCEL;
long motorAccelFork = FORK_MAX_ACCEL;
long motorSpeedPaintRotation = PAINT_ROTATION_MOTOR_SPEED;
long motorAccelPaintRotation = PAINT_ROTATION_MOTOR_ACCEL;
long storageMotorStepsPerClick = STORAGE_MOTOR_STEPS_PER_CLICK;
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
    if (motorPaintRotation) {
        motorPaintRotation->setSpeed(motorSpeedPaintRotation);
        motorPaintRotation->setAcceleration(motorAccelPaintRotation);
    }
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

    // Setup all API routes
    setupAPIRoutes();

    server.begin();
    Serial.println("Web Server initialized");
    Serial.println("Sensor Dashboard available at /");
}

void updateWebServer() {
    // Update Bounce2 sensors
    storagePositionSensor.update();
}
