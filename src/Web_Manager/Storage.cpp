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
    preferences.putInt("selectedColumn", selectedColumn);
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
    selectedColumn = preferences.getInt("selectedColumn", 0);  // Default to column A
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
    preferences.putLong("storageTrim", storageMotorTrimDistance);
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
    storageMotorTrimDistance = preferences.getLong("storageTrim", STORAGE_MOTOR_HOMING_TRIM);
    paintRotationMotorStepsPerClick = preferences.getLong("paintRotSteps", PAINT_ROTATION_MOTOR_STEPS_PER_CLICK);
    paintRotationMotorStepsPerRevOutput = preferences.getLong("paintRotRevOut", PAINT_ROTATION_MOTOR_STEPS_PER_REV_OUTPUT);
    servoSpeed = preferences.getFloat("servoSpeed", 30.0);
    preferences.end();
}

// Save square sensing toggle state to non-volatile storage
void saveSquareSensingState() {
    preferences.begin("testSeq", false);
    preferences.putBool("squareSensing", squareSensingEnabled);
    preferences.end();
}

// Load square sensing toggle state from non-volatile storage
void loadSquareSensingState() {
    preferences.begin("testSeq", true);
    squareSensingEnabled = preferences.getBool("squareSensing", true);  // Default to enabled
    preferences.end();
}

// Save painting sequence to non-volatile storage
void savePaintingSequence() {
    preferences.begin("paintSeq", false);
    preferences.putInt("count", paintingSequenceCount);
    for (int i = 0; i < paintingSequenceCount; i++) {
        String prefix = "b" + String(i) + "_";
        preferences.putInt((prefix + "type").c_str(), paintingSequence[i].type);
        preferences.putFloat((prefix + "p1").c_str(), paintingSequence[i].param1);
        preferences.putFloat((prefix + "p2").c_str(), paintingSequence[i].param2);
        preferences.putBool((prefix + "par").c_str(), paintingSequence[i].parallel);
    }
    preferences.end();
}

// Load painting sequence from non-volatile storage
void loadPaintingSequence() {
    preferences.begin("paintSeq", true);
    paintingSequenceCount = preferences.getInt("count", 0);
    if (paintingSequenceCount > MAX_PAINTING_BLOCKS) {
        paintingSequenceCount = MAX_PAINTING_BLOCKS;
    }
    for (int i = 0; i < paintingSequenceCount; i++) {
        String prefix = "b" + String(i) + "_";
        paintingSequence[i].type = preferences.getInt((prefix + "type").c_str(), 0);
        paintingSequence[i].param1 = preferences.getFloat((prefix + "p1").c_str(), 0.0);
        paintingSequence[i].param2 = preferences.getFloat((prefix + "p2").c_str(), 0.0);
        paintingSequence[i].parallel = preferences.getBool((prefix + "par").c_str(), false);
    }
    paintingSequenceLoaded = true;
    preferences.end();
}

