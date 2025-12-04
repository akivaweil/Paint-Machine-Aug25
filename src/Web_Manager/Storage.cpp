#include <Arduino.h>
#include <Preferences.h>
#include "Web_Manager.h"
#include "config/Config.h"

//* ************************************************************************
//* ************************ STORAGE **************************************
//* ************************************************************************

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
    preferences.putInt("pos1Height", selectedPosition1Height);
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
    selectedPosition1Height = preferences.getInt("pos1Height", 8);
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
    preferences.putLong("spdPaintRot", motorSpeedPaintRotation);
    preferences.putLong("accPaintRot", motorAccelPaintRotation);
    preferences.putLong("spdStorage", motorSpeedStorage);
    preferences.putLong("accStorage", motorAccelStorage);
    preferences.putLong("storageSteps", storageMotorStepsPerClick);
    preferences.putLong("paintRotSteps", paintRotationMotorStepsPerClick);
    preferences.putLong("paintRotRevOut", paintRotationMotorStepsPerRevOutput);
    preferences.putFloat("servoSpeed", servoSpeed);
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
    
    // Load paint rotation motor settings
    motorSpeedPaintRotation = preferences.getLong("spdPaintRot", PAINT_ROTATION_MOTOR_SPEED);
    motorAccelPaintRotation = preferences.getLong("accPaintRot", PAINT_ROTATION_MOTOR_ACCEL);
    
    // Load storage motor settings
    motorSpeedStorage = preferences.getLong("spdStorage", STORAGE_MOTOR_SPEED);
    motorAccelStorage = preferences.getLong("accStorage", STORAGE_MOTOR_ACCEL);
    
    storageMotorStepsPerClick = preferences.getLong("storageSteps", STORAGE_MOTOR_STEPS_PER_CLICK);
    paintRotationMotorStepsPerClick = preferences.getLong("paintRotSteps", PAINT_ROTATION_MOTOR_STEPS_PER_CLICK);
    paintRotationMotorStepsPerRevOutput = preferences.getLong("paintRotRevOut", PAINT_ROTATION_MOTOR_STEPS_PER_REV_OUTPUT);
    servoSpeed = preferences.getFloat("servoSpeed", 30.0);
    preferences.end();
}

