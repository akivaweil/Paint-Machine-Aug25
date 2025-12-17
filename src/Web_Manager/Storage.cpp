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

// Save painting configuration to non-volatile storage
void savePaintingConfig() {
    preferences.begin("paintCfg", false);
    preferences.putFloat("svHomeAng", cfgServoHomeAngle);
    preferences.putFloat("svPaintAng", cfgServoPaintingAngle);
    preferences.putULong("svUpdInt", cfgServoUpdateIntervalMs);
    preferences.putFloat("svTgtThr", cfgServoTargetReachedThresholdDeg);
    preferences.putFloat("svFarThr", cfgServoFarFromTargetThresholdDeg);
    preferences.putFloat("svMinMov", cfgServoMinMovementDeg);
    preferences.putFloat("svMaxStp", cfgServoMaxStepSizeDeg);
    preferences.putFloat("step1XOff", cfgStep1WaitingPositionXOffsetInches);
    preferences.putFloat("step1YOff", cfgStep1WaitingPositionYOffsetInches);
    preferences.putULong("step2Del", cfgStep2WaitingPositionDelayMs);
    preferences.putFloat("step3Fast", cfgStep3ServoFastSpeed);
    preferences.putULong("step4Del", cfgStep4InitialRotationDelayMs);
    preferences.putFloat("step4Rot", cfgStep4InitialRotationRev);
    preferences.putFloat("step5Rot", cfgStep5LeftRotationRev);
    preferences.putULong("step6Wait", cfgStep6LeftSideWaitMs);
    preferences.putFloat("step7Rot", cfgStep7BackLeftRotationRev);
    preferences.putULong("step8Wait", cfgStep8BackLeftWaitMs);
    preferences.putFloat("step8Ang", cfgStep8ServoBackAngleDeg);
    preferences.putFloat("step8Spd", cfgStep8BackLeftServoSpeed);
    preferences.putULong("step8Del", cfgStep8BackLeftPaintDelayMs);
    preferences.putFloat("step9Rot", cfgStep9BackRotationRev);
    preferences.putULong("step10Wait", cfgStep10BackSideWaitMs);
    preferences.putFloat("step10Ang", cfgStep10ServoBackAngleDeg);
    preferences.putFloat("step10Spd", cfgStep10BackServoSpeed);
    preferences.putULong("step10Del", cfgStep10BackPaintDelayMs);
    preferences.putFloat("step11Rot", cfgStep11BackRightRotationRev);
    preferences.putULong("step12Wait", cfgStep12BackRightWaitMs);
    preferences.putFloat("step12Ang", cfgStep12ServoBackAngleDeg);
    preferences.putFloat("step12Spd", cfgStep12BackRightServoSpeed);
    preferences.putULong("step12Del", cfgStep12BackRightPaintDelayMs);
    preferences.putFloat("step13Rot", cfgStep13RightRotationRev);
    preferences.putULong("step14Wait", cfgStep14RightSideWaitMs);
    preferences.putFloat("step14Ang", cfgStep14ServoRightAngleDeg);
    preferences.putFloat("step14Spd", cfgStep14RightServoSpeed);
    preferences.putULong("step14Del", cfgStep14RightPaintDelayMs);
    preferences.putULong("step15Del", cfgStep15PaintGunOffDelayMs);
    preferences.putFloat("step15Rot", cfgStep15FinalRotationRev);
    preferences.putFloat("step15Ang", cfgStep15FirstRevServoAngleDeg);
    preferences.putFloat("step16Ang", cfgStep16SpinServoAngleDeg);
    preferences.putULong("step18Del", cfgStep18PaintGunOffDelayMs);
    preferences.putFloat("step17Ang", cfgStep17InitialServoAngleDeg);
    preferences.putULong("step17Wait", cfgStep17InitialAngleWaitMs);
    preferences.putFloat("step17Spd", cfgStep17ServoSpeed);
    preferences.end();
}

// Load painting configuration from non-volatile storage
void loadPaintingConfig() {
    preferences.begin("paintCfg", true);
    cfgServoHomeAngle = preferences.getFloat("svHomeAng", SERVO_HOME_ANGLE);
    cfgServoPaintingAngle = preferences.getFloat("svPaintAng", SERVO_PAINTING_ANGLE);
    cfgServoUpdateIntervalMs = preferences.getULong("svUpdInt", SERVO_UPDATE_INTERVAL_MS);
    cfgServoTargetReachedThresholdDeg = preferences.getFloat("svTgtThr", SERVO_TARGET_REACHED_THRESHOLD_DEG);
    cfgServoFarFromTargetThresholdDeg = preferences.getFloat("svFarThr", SERVO_FAR_FROM_TARGET_THRESHOLD_DEG);
    cfgServoMinMovementDeg = preferences.getFloat("svMinMov", SERVO_MIN_MOVEMENT_DEG);
    cfgServoMaxStepSizeDeg = preferences.getFloat("svMaxStp", SERVO_MAX_STEP_SIZE_DEG);
    cfgStep1WaitingPositionXOffsetInches = preferences.getFloat("step1XOff", STEP1_WAITING_POSITION_X_OFFSET_INCHES);
    cfgStep1WaitingPositionYOffsetInches = preferences.getFloat("step1YOff", STEP1_WAITING_POSITION_Y_OFFSET_INCHES);
    cfgStep2WaitingPositionDelayMs = preferences.getULong("step2Del", STEP2_WAITING_POSITION_DELAY_MS);
    cfgStep3ServoFastSpeed = preferences.getFloat("step3Fast", STEP3_SERVO_FAST_SPEED);
    cfgStep4InitialRotationDelayMs = preferences.getULong("step4Del", STEP4_INITIAL_ROTATION_DELAY_MS);
    cfgStep4InitialRotationRev = preferences.getFloat("step4Rot", STEP4_INITIAL_ROTATION_REV);
    cfgStep5LeftRotationRev = preferences.getFloat("step5Rot", STEP5_LEFT_ROTATION_REV);
    cfgStep6LeftSideWaitMs = preferences.getULong("step6Wait", STEP6_LEFT_SIDE_WAIT_MS);
    cfgStep7BackLeftRotationRev = preferences.getFloat("step7Rot", STEP7_BACK_LEFT_ROTATION_REV);
    cfgStep8BackLeftWaitMs = preferences.getULong("step8Wait", STEP8_BACK_LEFT_WAIT_MS);
    cfgStep8ServoBackAngleDeg = preferences.getFloat("step8Ang", STEP8_SERVO_BACK_ANGLE_DEG);
    cfgStep8BackLeftServoSpeed = preferences.getFloat("step8Spd", STEP8_BACK_LEFT_SERVO_SPEED);
    cfgStep8BackLeftPaintDelayMs = preferences.getULong("step8Del", STEP8_BACK_LEFT_PAINT_DELAY_MS);
    cfgStep9BackRotationRev = preferences.getFloat("step9Rot", STEP9_BACK_ROTATION_REV);
    cfgStep10BackSideWaitMs = preferences.getULong("step10Wait", STEP10_BACK_SIDE_WAIT_MS);
    cfgStep10ServoBackAngleDeg = preferences.getFloat("step10Ang", STEP10_SERVO_BACK_ANGLE_DEG);
    cfgStep10BackServoSpeed = preferences.getFloat("step10Spd", STEP10_BACK_SERVO_SPEED);
    cfgStep10BackPaintDelayMs = preferences.getULong("step10Del", STEP10_BACK_PAINT_DELAY_MS);
    cfgStep11BackRightRotationRev = preferences.getFloat("step11Rot", STEP11_BACK_RIGHT_ROTATION_REV);
    cfgStep12BackRightWaitMs = preferences.getULong("step12Wait", STEP12_BACK_RIGHT_WAIT_MS);
    cfgStep12ServoBackAngleDeg = preferences.getFloat("step12Ang", STEP12_SERVO_BACK_ANGLE_DEG);
    cfgStep12BackRightServoSpeed = preferences.getFloat("step12Spd", STEP12_BACK_RIGHT_SERVO_SPEED);
    cfgStep12BackRightPaintDelayMs = preferences.getULong("step12Del", STEP12_BACK_RIGHT_PAINT_DELAY_MS);
    cfgStep13RightRotationRev = preferences.getFloat("step13Rot", STEP13_RIGHT_ROTATION_REV);
    cfgStep14RightSideWaitMs = preferences.getULong("step14Wait", STEP14_RIGHT_SIDE_WAIT_MS);
    cfgStep14ServoRightAngleDeg = preferences.getFloat("step14Ang", STEP14_SERVO_RIGHT_ANGLE_DEG);
    cfgStep14RightServoSpeed = preferences.getFloat("step14Spd", STEP14_RIGHT_SERVO_SPEED);
    cfgStep14RightPaintDelayMs = preferences.getULong("step14Del", STEP14_RIGHT_PAINT_DELAY_MS);
    cfgStep15PaintGunOffDelayMs = preferences.getULong("step15Del", STEP15_PAINT_GUN_OFF_DELAY_MS);
    cfgStep15FinalRotationRev = preferences.getFloat("step15Rot", STEP15_FINAL_ROTATION_REV);
    cfgStep15FirstRevServoAngleDeg = preferences.getFloat("step15Ang", STEP15_FIRST_REV_SERVO_ANGLE_DEG);
    cfgStep16SpinServoAngleDeg = preferences.getFloat("step16Ang", STEP16_SPIN_SERVO_ANGLE_DEG);
    cfgStep18PaintGunOffDelayMs = preferences.getULong("step18Del", STEP18_PAINT_GUN_OFF_DELAY_MS);
    cfgStep17InitialServoAngleDeg = preferences.getFloat("step17Ang", STEP17_INITIAL_SERVO_ANGLE_DEG);
    cfgStep17InitialAngleWaitMs = preferences.getULong("step17Wait", STEP17_INITIAL_ANGLE_WAIT_MS);
    cfgStep17ServoSpeed = preferences.getFloat("step17Spd", STEP17_SERVO_SPEED);
    preferences.end();
}

