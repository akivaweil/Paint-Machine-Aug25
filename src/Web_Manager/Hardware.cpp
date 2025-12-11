#include <Arduino.h>
#include <Bounce2.h>
#include "Web_Manager.h"
#include "config/Pin_Definitions.h"
#include "config/Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/StorageMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"
#include "ServoControl.h"
#include "Paint_Motor_Controller.h"

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
    
    // Initialize Ultrasonic Sensor
    pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
    pinMode(ULTRASONIC_ECHO_PIN, INPUT);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    
    // Initialize Servo
    if (servo == nullptr) {
        servo = new ServoControl();
        servo->init(SERVO_PIN, 0, 50, 16);  // pin, channel, frequency, resolution
        servo->setAngleRange(0, 270);  // Set to 270 degree range
        currentServoAngle = SERVO_HOME_ANGLE;  // Initialize current position
        servo->write(SERVO_HOME_ANGLE);  // Set to servo home angle (default/idle position)
    }
    
    // Initialize Suction and Paint Gun pins as outputs
    pinMode(SUCTION_PIN, OUTPUT);
    pinMode(PAINT_GUN_PIN, OUTPUT);
    pinMode(PRESSURE_POT_PIN, OUTPUT);
    digitalWrite(SUCTION_PIN, LOW);
    digitalWrite(PAINT_GUN_PIN, LOW);
    digitalWrite(PRESSURE_POT_PIN, LOW);
    suctionState = false;
    paintGunState = false;
    pressurePotState = false;
    
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
    enableStepper();
}

void disablePaintRotationMotor() {
    disableStepper();
}

bool isPaintRotationMotorRunning() {
    return isStepperRunning();
}

// Check if square is present using ultrasonic sensor
// Returns true if distance is less than threshold (square present)
bool isSquarePresent() {
    // Clear trigger pin
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    
    // Send 10us trigger pulse
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    
    // Read echo pulse duration (timeout after 30000us = ~500cm max range)
    long pulseDuration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH, 30000);
    
    // Calculate distance in cm
    // Speed of sound = 343 m/s = 0.034 cm/us
    // Distance = (pulse duration * speed) / 2 (divide by 2 because sound travels to object and back)
    float distanceCm = (pulseDuration * 0.034) / 2.0;
    
    // Return true if distance is less than threshold (square present)
    return (distanceCm > 0 && distanceCm < ULTRASONIC_SQUARE_PRESENT_THRESHOLD_CM);
}

// Initialize motors
void initializeMotors() {
    // Initialize paint rotation motor controller FIRST to ensure it gets resources
    initializeStepper();
    disablePaintRotationMotor();  // Start with motor disabled

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
        motorStorage = new StorageMotor(STORAGE_STEP_PIN, STORAGE_DIR_PIN, STEPS_PER_INCH, motorSpeedStorage, motorAccelStorage);
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
    bool squarePresent = isSquarePresent(); // Ultrasonic sensor - true if distance < threshold
    
    String json = "{";
    json += "\"xHome1\":" + String(xHome1 ? "true" : "false") + ",";
    json += "\"xHome2\":" + String(xHome2 ? "true" : "false") + ",";
    json += "\"yHome\":" + String(yHome ? "true" : "false") + ",";
    json += "\"forkHome\":" + String(forkHome ? "true" : "false") + ",";
    json += "\"testButton\":" + String(testButton ? "true" : "false") + ",";
    json += "\"storagePosition\":" + String(storagePosition ? "true" : "false") + ",";
    json += "\"squarePresent\":" + String(squarePresent ? "true" : "false");
    json += "}";
    
    return json;
}

