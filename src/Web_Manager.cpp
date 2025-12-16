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
// -1 indicates storage has not been homed
int currentColumn = -1;  // Current column position (set during homing, -1 = not homed)
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

// Skip painting toggle (skips painting state entirely during cycle)
bool skipPaintingEnabled = false;  // Default to disabled

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

// Painting configuration settings (runtime adjustable)
float paintingConfigServoHomeAngle = SERVO_HOME_ANGLE;
float paintingConfigServoPaintingAngle = SERVO_PAINTING_ANGLE;
unsigned long paintingConfigServoUpdateIntervalMs = SERVO_UPDATE_INTERVAL_MS;
float paintingConfigServoTargetReachedThresholdDeg = SERVO_TARGET_REACHED_THRESHOLD_DEG;
float paintingConfigServoFarFromTargetThresholdDeg = SERVO_FAR_FROM_TARGET_THRESHOLD_DEG;
float paintingConfigServoMinMovementDeg = SERVO_MIN_MOVEMENT_DEG;
float paintingConfigServoMaxStepSizeDeg = SERVO_MAX_STEP_SIZE_DEG;
float paintingConfigStep1WaitingPositionXOffsetInches = STEP1_WAITING_POSITION_X_OFFSET_INCHES;
float paintingConfigStep1WaitingPositionYOffsetInches = STEP1_WAITING_POSITION_Y_OFFSET_INCHES;
unsigned long paintingConfigStep2WaitingPositionDelayMs = STEP2_WAITING_POSITION_DELAY_MS;
float paintingConfigStep3ServoFastSpeed = STEP3_SERVO_FAST_SPEED;
unsigned long paintingConfigStep4InitialRotationDelayMs = STEP4_INITIAL_ROTATION_DELAY_MS;
float paintingConfigStep4InitialRotationRev = STEP4_INITIAL_ROTATION_REV;
float paintingConfigStep5LeftRotationRev = STEP5_LEFT_ROTATION_REV;
unsigned long paintingConfigStep6LeftSideWaitMs = STEP6_LEFT_SIDE_WAIT_MS;
float paintingConfigStep7BackLeftRotationRev = STEP7_BACK_LEFT_ROTATION_REV;
unsigned long paintingConfigStep8BackLeftWaitMs = STEP8_BACK_LEFT_WAIT_MS;
float paintingConfigStep8ServoBackAngleDeg = STEP8_SERVO_BACK_ANGLE_DEG;
float paintingConfigStep8BackLeftServoSpeed = STEP8_BACK_LEFT_SERVO_SPEED;
unsigned long paintingConfigStep8BackLeftPaintDelayMs = STEP8_BACK_LEFT_PAINT_DELAY_MS;
float paintingConfigStep9BackRotationRev = STEP9_BACK_ROTATION_REV;
unsigned long paintingConfigStep10BackSideWaitMs = STEP10_BACK_SIDE_WAIT_MS;
float paintingConfigStep10ServoBackAngleDeg = STEP10_SERVO_BACK_ANGLE_DEG;
float paintingConfigStep10BackServoSpeed = STEP10_BACK_SERVO_SPEED;
unsigned long paintingConfigStep10BackPaintDelayMs = STEP10_BACK_PAINT_DELAY_MS;
float paintingConfigStep11BackRightRotationRev = STEP11_BACK_RIGHT_ROTATION_REV;
unsigned long paintingConfigStep12BackRightWaitMs = STEP12_BACK_RIGHT_WAIT_MS;
float paintingConfigStep12ServoBackAngleDeg = STEP12_SERVO_BACK_ANGLE_DEG;
float paintingConfigStep12BackRightServoSpeed = STEP12_BACK_RIGHT_SERVO_SPEED;
unsigned long paintingConfigStep12BackRightPaintDelayMs = STEP12_BACK_RIGHT_PAINT_DELAY_MS;
float paintingConfigStep13RightRotationRev = STEP13_RIGHT_ROTATION_REV;
unsigned long paintingConfigStep14RightSideWaitMs = STEP14_RIGHT_SIDE_WAIT_MS;
float paintingConfigStep14ServoRightAngleDeg = STEP14_SERVO_RIGHT_ANGLE_DEG;
float paintingConfigStep14RightServoSpeed = STEP14_RIGHT_SERVO_SPEED;
unsigned long paintingConfigStep14RightPaintDelayMs = STEP14_RIGHT_PAINT_DELAY_MS;
unsigned long paintingConfigStep15PaintGunOffDelayMs = STEP15_PAINT_GUN_OFF_DELAY_MS;
float paintingConfigStep15FinalRotationRev = STEP15_FINAL_ROTATION_REV;
float paintingConfigStep15FirstRevServoAngleDeg = STEP15_FIRST_REV_SERVO_ANGLE_DEG;
float paintingConfigStep16SpinServoAngleDeg = STEP16_SPIN_SERVO_ANGLE_DEG;
unsigned long paintingConfigStep18PaintGunOffDelayMs = STEP18_PAINT_GUN_OFF_DELAY_MS;
float paintingConfigStep17InitialServoAngleDeg = STEP17_INITIAL_SERVO_ANGLE_DEG;
unsigned long paintingConfigStep17InitialAngleWaitMs = STEP17_INITIAL_ANGLE_WAIT_MS;
float paintingConfigStep17ServoSpeed = STEP17_SERVO_SPEED;

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
    
    // Load saved skip painting state
    loadSkipPaintingState();
    
    // Load saved painting config
    loadPaintingConfig();
    
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
