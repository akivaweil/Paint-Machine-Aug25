#include <Arduino.h>
#include <Bounce2.h>
#include "Web_Manager.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"
#include "ServoControl.h"

//* ************************************************************************
//* ************************ HARDWARE *************************************
//* ************************************************************************

// Initialize sensor pins
void initializeSensors() {
    if (sensorsInitialized) return;
    
    // Initialize test button (active HIGH with pulldown)
    pinMode(TEST_BUTTON_PIN, INPUT_PULLDOWN);
    
    // Initialize Storage Position Sensor (active HIGH with pulldown, 5ms debounce)
    storagePositionSensor.attach(STORAGE_POSITION_SENSOR_PIN, INPUT_PULLDOWN);
    storagePositionSensor.interval(5);  // 5ms debounce
    
    // Initialize Servo
    if (servo == nullptr) {
        servo = new ServoControl();
        servo->init(SERVO_PIN, 0, 50, 16);  // pin, channel, frequency, resolution
        servo->setAngleRange(0, 270);  // Set to 270 degree range
        currentServoAngle = SERVO_HOME_ANGLE;  // Initialize current position
        servo->write(SERVO_HOME_ANGLE);  // Set to servo home angle (default position)
    }
    
    // Initialize Suction and Paint Gun pins as outputs
    pinMode(SUCTION_PIN, OUTPUT);
    pinMode(PAINT_GUN_PIN, OUTPUT);
    digitalWrite(SUCTION_PIN, LOW);
    digitalWrite(PAINT_GUN_PIN, LOW);
    suctionState = false;
    paintGunState = false;
    
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

// Set servo angle (0-270 degrees) - moves gradually at configurable speed
void setServoAngle(float angle) {
    if (servo) {
        // Constrain angle to valid range
        if (angle < 0) angle = 0;
        if (angle > 270) angle = 270;
        
        // Calculate step size and delay based on configured speed
        // servoSpeed is in degrees per second
        const float stepSize = 0.5;  // Step size in degrees
        const float stepDelayMs = (stepSize / servoSpeed) * 1000.0;  // Delay in milliseconds
        
        // Move gradually from current to target
        float targetAngle = angle;
        float diff = targetAngle - currentServoAngle;
        
        if (abs(diff) > stepSize) {
            // Move in steps
            int steps = abs(diff) / stepSize;
            float increment = diff / steps;
            
            for (int i = 0; i < steps; i++) {
                currentServoAngle += increment;
                servo->write(currentServoAngle);
                delay((int)stepDelayMs);
            }
        }
        
        // Final position to ensure accuracy
        currentServoAngle = targetAngle;
        servo->write(currentServoAngle);
    }
}

// Enable/disable paint rotation motor (enable pin is active LOW)
void enablePaintRotationMotor() {
    digitalWrite(PAINT_ROTATION_ENABLE_PIN, LOW);  // LOW = enabled
}

void disablePaintRotationMotor() {
    digitalWrite(PAINT_ROTATION_ENABLE_PIN, HIGH);  // HIGH = disabled
}

bool isPaintRotationMotorRunning() {
    if (motorPaintRotation) {
        return motorPaintRotation->isMotorRunning();
    }
    return false;
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
        motorStorage = new StepperMotor(STORAGE_STEP_PIN, STORAGE_DIR_PIN, STEPS_PER_INCH, motorSpeedStorage, motorAccelStorage);
    }
    if (motorPaintRotation == nullptr) {
        motorPaintRotation = new StepperMotor(PAINT_ROTATION_STEP_PIN, PAINT_ROTATION_DIR_PIN, STEPS_PER_INCH, motorSpeedPaintRotation, motorAccelPaintRotation);
        // Initialize enable pin for paint rotation motor
        pinMode(PAINT_ROTATION_ENABLE_PIN, OUTPUT);
        disablePaintRotationMotor();  // Start with motor disabled
    }
}

// Read sensor states
String getSensorStatesJSON() {
    // Update Bounce2 sensor
    storagePositionSensor.update();
    
    // Read X home switches (need individual pins for JSON)
    bool xHome1 = homeSwitchX ? digitalRead(X_HOME_PIN) : false;
    bool xHome2 = homeSwitchX ? digitalRead(X_HOME_PIN2) : false;
    // Read Y and Fork using HomeSwitch instances
    bool yHome = homeSwitchY ? homeSwitchY->read() : false;
    bool forkHome = homeSwitchFork ? homeSwitchFork->read() : false;
    bool testButton = digitalRead(TEST_BUTTON_PIN); // Active HIGH
    bool storagePosition = storagePositionSensor.read(); // Active HIGH
    
    String json = "{";
    json += "\"xHome1\":" + String(xHome1 ? "true" : "false") + ",";
    json += "\"xHome2\":" + String(xHome2 ? "true" : "false") + ",";
    json += "\"yHome\":" + String(yHome ? "true" : "false") + ",";
    json += "\"forkHome\":" + String(forkHome ? "true" : "false") + ",";
    json += "\"testButton\":" + String(testButton ? "true" : "false") + ",";
    json += "\"storagePosition\":" + String(storagePosition ? "true" : "false");
    json += "}";
    
    return json;
}

