#include <Arduino.h>
#include <Preferences.h>
#include "Web_Manager.h"
#include "config/Config.h"
#include "config/Painting_Config.h"

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

// Save test mode toggle state to non-volatile storage
void saveTestModeState() {
    preferences.begin("testSeq", false);
    preferences.putBool("testMode", testModeEnabled);
    preferences.end();
}

// Load test mode toggle state from non-volatile storage
void loadTestModeState() {
    preferences.begin("testSeq", true);
    testModeEnabled = preferences.getBool("testMode", false);  // Default to disabled
    preferences.end();
}

// Save skip painting toggle state to non-volatile storage
void saveSkipPaintingState() {
    preferences.begin("testSeq", false);
    preferences.putBool("skipPainting", skipPaintingEnabled);
    preferences.end();
}

// Load skip painting toggle state from non-volatile storage
void loadSkipPaintingState() {
    preferences.begin("testSeq", true);
    skipPaintingEnabled = preferences.getBool("skipPainting", false);  // Default to disabled
    preferences.end();
}

// Save painting configuration settings to non-volatile storage
void savePaintingConfig() {
    preferences.begin("paintConfig", false);
    preferences.putFloat("servoHomeAngle", paintingConfigServoHomeAngle);
    preferences.putFloat("servoPaintingAngle", paintingConfigServoPaintingAngle);
    preferences.putULong("servoUpdateIntervalMs", paintingConfigServoUpdateIntervalMs);
    preferences.putFloat("servoTargetReachedThresholdDeg", paintingConfigServoTargetReachedThresholdDeg);
    preferences.putFloat("servoFarFromTargetThresholdDeg", paintingConfigServoFarFromTargetThresholdDeg);
    preferences.putFloat("servoMinMovementDeg", paintingConfigServoMinMovementDeg);
    preferences.putFloat("servoMaxStepSizeDeg", paintingConfigServoMaxStepSizeDeg);
    preferences.putFloat("step1WaitingPositionXOffsetInches", paintingConfigStep1WaitingPositionXOffsetInches);
    preferences.putFloat("step1WaitingPositionYOffsetInches", paintingConfigStep1WaitingPositionYOffsetInches);
    preferences.putULong("step2WaitingPositionDelayMs", paintingConfigStep2WaitingPositionDelayMs);
    preferences.putFloat("step3ServoFastSpeed", paintingConfigStep3ServoFastSpeed);
    preferences.putULong("step4InitialRotationDelayMs", paintingConfigStep4InitialRotationDelayMs);
    preferences.putFloat("step4InitialRotationRev", paintingConfigStep4InitialRotationRev);
    preferences.putFloat("step5LeftRotationRev", paintingConfigStep5LeftRotationRev);
    preferences.putULong("step6LeftSideWaitMs", paintingConfigStep6LeftSideWaitMs);
    preferences.putFloat("step7BackLeftRotationRev", paintingConfigStep7BackLeftRotationRev);
    preferences.putULong("step8BackLeftWaitMs", paintingConfigStep8BackLeftWaitMs);
    preferences.putFloat("step8ServoBackAngleDeg", paintingConfigStep8ServoBackAngleDeg);
    preferences.putFloat("step8BackLeftServoSpeed", paintingConfigStep8BackLeftServoSpeed);
    preferences.putULong("step8BackLeftPaintDelayMs", paintingConfigStep8BackLeftPaintDelayMs);
    preferences.putFloat("step9BackRotationRev", paintingConfigStep9BackRotationRev);
    preferences.putULong("step10BackSideWaitMs", paintingConfigStep10BackSideWaitMs);
    preferences.putFloat("step10ServoBackAngleDeg", paintingConfigStep10ServoBackAngleDeg);
    preferences.putFloat("step10BackServoSpeed", paintingConfigStep10BackServoSpeed);
    preferences.putULong("step10BackPaintDelayMs", paintingConfigStep10BackPaintDelayMs);
    preferences.putFloat("step11BackRightRotationRev", paintingConfigStep11BackRightRotationRev);
    preferences.putULong("step12BackRightWaitMs", paintingConfigStep12BackRightWaitMs);
    preferences.putFloat("step12ServoBackAngleDeg", paintingConfigStep12ServoBackAngleDeg);
    preferences.putFloat("step12BackRightServoSpeed", paintingConfigStep12BackRightServoSpeed);
    preferences.putULong("step12BackRightPaintDelayMs", paintingConfigStep12BackRightPaintDelayMs);
    preferences.putFloat("step13RightRotationRev", paintingConfigStep13RightRotationRev);
    preferences.putULong("step14RightSideWaitMs", paintingConfigStep14RightSideWaitMs);
    preferences.putFloat("step14ServoRightAngleDeg", paintingConfigStep14ServoRightAngleDeg);
    preferences.putFloat("step14RightServoSpeed", paintingConfigStep14RightServoSpeed);
    preferences.putULong("step14RightPaintDelayMs", paintingConfigStep14RightPaintDelayMs);
    preferences.putULong("step15PaintGunOffDelayMs", paintingConfigStep15PaintGunOffDelayMs);
    preferences.putFloat("step15FinalRotationRev", paintingConfigStep15FinalRotationRev);
    preferences.putFloat("step15FirstRevServoAngleDeg", paintingConfigStep15FirstRevServoAngleDeg);
    preferences.putFloat("step16SpinServoAngleDeg", paintingConfigStep16SpinServoAngleDeg);
    preferences.putULong("step18PaintGunOffDelayMs", paintingConfigStep18PaintGunOffDelayMs);
    preferences.putFloat("step17InitialServoAngleDeg", paintingConfigStep17InitialServoAngleDeg);
    preferences.putULong("step17InitialAngleWaitMs", paintingConfigStep17InitialAngleWaitMs);
    preferences.putFloat("step17ServoSpeed", paintingConfigStep17ServoSpeed);
    preferences.end();
}

// Load painting configuration settings from non-volatile storage
void loadPaintingConfig() {
    preferences.begin("paintConfig", true);
    paintingConfigServoHomeAngle = preferences.getFloat("servoHomeAngle", SERVO_HOME_ANGLE);
    paintingConfigServoPaintingAngle = preferences.getFloat("servoPaintingAngle", SERVO_PAINTING_ANGLE);
    paintingConfigServoUpdateIntervalMs = preferences.getULong("servoUpdateIntervalMs", SERVO_UPDATE_INTERVAL_MS);
    paintingConfigServoTargetReachedThresholdDeg = preferences.getFloat("servoTargetReachedThresholdDeg", SERVO_TARGET_REACHED_THRESHOLD_DEG);
    paintingConfigServoFarFromTargetThresholdDeg = preferences.getFloat("servoFarFromTargetThresholdDeg", SERVO_FAR_FROM_TARGET_THRESHOLD_DEG);
    paintingConfigServoMinMovementDeg = preferences.getFloat("servoMinMovementDeg", SERVO_MIN_MOVEMENT_DEG);
    paintingConfigServoMaxStepSizeDeg = preferences.getFloat("servoMaxStepSizeDeg", SERVO_MAX_STEP_SIZE_DEG);
    paintingConfigStep1WaitingPositionXOffsetInches = preferences.getFloat("step1WaitingPositionXOffsetInches", STEP1_WAITING_POSITION_X_OFFSET_INCHES);
    paintingConfigStep1WaitingPositionYOffsetInches = preferences.getFloat("step1WaitingPositionYOffsetInches", STEP1_WAITING_POSITION_Y_OFFSET_INCHES);
    paintingConfigStep2WaitingPositionDelayMs = preferences.getULong("step2WaitingPositionDelayMs", STEP2_WAITING_POSITION_DELAY_MS);
    paintingConfigStep3ServoFastSpeed = preferences.getFloat("step3ServoFastSpeed", STEP3_SERVO_FAST_SPEED);
    paintingConfigStep4InitialRotationDelayMs = preferences.getULong("step4InitialRotationDelayMs", STEP4_INITIAL_ROTATION_DELAY_MS);
    paintingConfigStep4InitialRotationRev = preferences.getFloat("step4InitialRotationRev", STEP4_INITIAL_ROTATION_REV);
    paintingConfigStep5LeftRotationRev = preferences.getFloat("step5LeftRotationRev", STEP5_LEFT_ROTATION_REV);
    paintingConfigStep6LeftSideWaitMs = preferences.getULong("step6LeftSideWaitMs", STEP6_LEFT_SIDE_WAIT_MS);
    paintingConfigStep7BackLeftRotationRev = preferences.getFloat("step7BackLeftRotationRev", STEP7_BACK_LEFT_ROTATION_REV);
    paintingConfigStep8BackLeftWaitMs = preferences.getULong("step8BackLeftWaitMs", STEP8_BACK_LEFT_WAIT_MS);
    paintingConfigStep8ServoBackAngleDeg = preferences.getFloat("step8ServoBackAngleDeg", STEP8_SERVO_BACK_ANGLE_DEG);
    paintingConfigStep8BackLeftServoSpeed = preferences.getFloat("step8BackLeftServoSpeed", STEP8_BACK_LEFT_SERVO_SPEED);
    paintingConfigStep8BackLeftPaintDelayMs = preferences.getULong("step8BackLeftPaintDelayMs", STEP8_BACK_LEFT_PAINT_DELAY_MS);
    paintingConfigStep9BackRotationRev = preferences.getFloat("step9BackRotationRev", STEP9_BACK_ROTATION_REV);
    paintingConfigStep10BackSideWaitMs = preferences.getULong("step10BackSideWaitMs", STEP10_BACK_SIDE_WAIT_MS);
    paintingConfigStep10ServoBackAngleDeg = preferences.getFloat("step10ServoBackAngleDeg", STEP10_SERVO_BACK_ANGLE_DEG);
    paintingConfigStep10BackServoSpeed = preferences.getFloat("step10BackServoSpeed", STEP10_BACK_SERVO_SPEED);
    paintingConfigStep10BackPaintDelayMs = preferences.getULong("step10BackPaintDelayMs", STEP10_BACK_PAINT_DELAY_MS);
    paintingConfigStep11BackRightRotationRev = preferences.getFloat("step11BackRightRotationRev", STEP11_BACK_RIGHT_ROTATION_REV);
    paintingConfigStep12BackRightWaitMs = preferences.getULong("step12BackRightWaitMs", STEP12_BACK_RIGHT_WAIT_MS);
    paintingConfigStep12ServoBackAngleDeg = preferences.getFloat("step12ServoBackAngleDeg", STEP12_SERVO_BACK_ANGLE_DEG);
    paintingConfigStep12BackRightServoSpeed = preferences.getFloat("step12BackRightServoSpeed", STEP12_BACK_RIGHT_SERVO_SPEED);
    paintingConfigStep12BackRightPaintDelayMs = preferences.getULong("step12BackRightPaintDelayMs", STEP12_BACK_RIGHT_PAINT_DELAY_MS);
    paintingConfigStep13RightRotationRev = preferences.getFloat("step13RightRotationRev", STEP13_RIGHT_ROTATION_REV);
    paintingConfigStep14RightSideWaitMs = preferences.getULong("step14RightSideWaitMs", STEP14_RIGHT_SIDE_WAIT_MS);
    paintingConfigStep14ServoRightAngleDeg = preferences.getFloat("step14ServoRightAngleDeg", STEP14_SERVO_RIGHT_ANGLE_DEG);
    paintingConfigStep14RightServoSpeed = preferences.getFloat("step14RightServoSpeed", STEP14_RIGHT_SERVO_SPEED);
    paintingConfigStep14RightPaintDelayMs = preferences.getULong("step14RightPaintDelayMs", STEP14_RIGHT_PAINT_DELAY_MS);
    paintingConfigStep15PaintGunOffDelayMs = preferences.getULong("step15PaintGunOffDelayMs", STEP15_PAINT_GUN_OFF_DELAY_MS);
    paintingConfigStep15FinalRotationRev = preferences.getFloat("step15FinalRotationRev", STEP15_FINAL_ROTATION_REV);
    paintingConfigStep15FirstRevServoAngleDeg = preferences.getFloat("step15FirstRevServoAngleDeg", STEP15_FIRST_REV_SERVO_ANGLE_DEG);
    paintingConfigStep16SpinServoAngleDeg = preferences.getFloat("step16SpinServoAngleDeg", STEP16_SPIN_SERVO_ANGLE_DEG);
    paintingConfigStep18PaintGunOffDelayMs = preferences.getULong("step18PaintGunOffDelayMs", STEP18_PAINT_GUN_OFF_DELAY_MS);
    paintingConfigStep17InitialServoAngleDeg = preferences.getFloat("step17InitialServoAngleDeg", STEP17_INITIAL_SERVO_ANGLE_DEG);
    paintingConfigStep17InitialAngleWaitMs = preferences.getULong("step17InitialAngleWaitMs", STEP17_INITIAL_ANGLE_WAIT_MS);
    paintingConfigStep17ServoSpeed = preferences.getFloat("step17ServoSpeed", STEP17_SERVO_SPEED);
    preferences.end();
}

