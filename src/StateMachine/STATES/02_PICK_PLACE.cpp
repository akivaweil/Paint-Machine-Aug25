#include <Arduino.h>
#include "StateMachine/STATES/02_PICK_PLACE.h"
#include "../../config/Config.h"
#include "../../config/Painting_Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "Web_Manager.h"
#include "ServoControl.h"
#include "../../config/Pin_Definitions.h"
#include "Paint_Motor_Controller.h"

// External motor instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;

// Home switch for fork motor
extern HomeSwitch* homeSwitchFork;

// External servo and paint gun controls
extern ServoControl* servo;
extern float currentServoAngle;
extern float servoSpeed;
extern void enablePaintRotationMotor();
extern void disablePaintRotationMotor();

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_IDLE 1
#define STATE_PAINTING 3

// Cycle control flags
extern bool cyclePaused;
extern bool cycleCancelled;

// Homing function
extern void homeAllAxes(bool resetColumn);

// Macro to check for pause after step completion
#define CHECK_PAUSE_AND_CANCEL() \
    do { \
        while (cyclePaused && !cycleCancelled) { \
            updateOTA(); \
            delay(10); \
        } \
        if (cycleCancelled) return; \
    } while(0)

// Test position values (set from web interface)
extern float testPos1X;
extern float testPos1Y;
extern float testPos1Fork;
extern float testPos2X;
extern float testPos2Y;
extern float testPos2Fork;
extern int selectedPosition1Height;  // Position 1 height selection (1-8, where 8 = a8/lowest)
extern int selectedColumn;  // Selected column for test cycle (0-5, where 0=A, 5=F)
extern bool testAllMode;  // Flag to track if we're in "test all" mode
extern int currentTestAllHeight;  // Track which height we're currently testing (1-8)
extern int testAllColumnCount;  // Number of columns to test (1-6)
extern int testAllStartColumn;  // Starting column position (from physical position)
extern int testAllCurrentColumnIndex;  // Current column index in the test sequence (0 to testAllColumnCount-1)

// Paint gun pin
#include "../../config/Pin_Definitions.h"

//* ************************************************************************
//* ************************ PICK AND PLACE STATE *************************
//* ************************************************************************

// Helper function to move Y-axis with speed adjustment based on direction
static void moveYWithSpeedAdjustment(float inches) {
    if (!motorY) return;
    
    extern long motorSpeedY;
    
    // Negative movement = up (away from home), positive = down (toward home)
    if (inches < 0) {
        // Moving up - use reduced speed to prevent stalling against gravity
        long upSpeed = (long)(motorSpeedY * Y_SPEED_UP_MULTIPLIER);
        motorY->setSpeed(upSpeed);
    } else {
        // Moving down - use normal speed
        motorY->setSpeed(motorSpeedY);
    }
    
    motorY->moveInches(inches);
}

void pickPlaceState() {
    static int step = 0;
    static bool testStarted = false;
    
    // Check for cancel at start of function
    if (cycleCancelled) {
        // Cleanup: stop all motors immediately
        if (motorX) motorX->forceStop();
        if (motorY) motorY->forceStop();
        if (motorFork) motorFork->forceStop();
        stopStepper();
        disablePaintRotationMotor();
        
        // Turn off paint gun and suction
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        // Immediately move servo to home angle
        if (servo) {
            currentServoAngle = SERVO_HOME_ANGLE;
            servo->write(SERVO_HOME_ANGLE);
        }
        
        // Home all motors (preserve column position)
        homeAllAxes(false);
        
        // Reset flags and state
        cycleCancelled = false;
        cyclePaused = false;
        testStarted = false;
        step = 0;
        
        // Return to idle
        setMachineState(STATE_IDLE);
        return;
    }
    
    // Reset state if we're entering after a cancel (testStarted might still be true from previous cycle)
    // Check if we're actually at home position to determine if we should reset
    if (testStarted) {
        // If motors are at home (position near 0), we're not in the middle of a cycle - reset
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // If all axes are near home (within 0.5 inches), reset the state
        // This catches cases where we cancelled and homed, but testStarted wasn't reset
        if (fabs(currentX) < 0.5 && fabs(currentY) < 0.5 && fabs(currentFork) < 0.5) {
            testStarted = false;
            step = 0;
        }
    }
    
    // Initialize on first entry
    if (!testStarted) {
        step = 0;
        testStarted = true;
        cyclePaused = false;  // Reset pause flag on new test
        cycleCancelled = false;  // Reset cancel flag on new test
        
        // Apply motor settings from dashboard before starting test
        applyMotorSettings();
        
        // Turn on pressure pot when starting run cycle (unless test mode is enabled)
        extern bool testModeEnabled;
        if (!testModeEnabled) {
            digitalWrite(PRESSURE_POT_PIN, HIGH);
            pressurePotState = true;
        }
        
        //! ************************************************************************
        //! STEP 0: MOVE STORAGE MOTOR TO SELECTED COLUMN
        //! ************************************************************************
        moveToColumn(selectedColumn);
        
        // Move servo to servo home angle at the beginning
        if (servo) {
            currentServoAngle = SERVO_HOME_ANGLE;
            servo->write(SERVO_HOME_ANGLE);
        }
    }
    
    //! ************************************************************************
    //! STEP 1: MOVE TO POSITION 1
    //! ************************************************************************
    if (step == 0) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate actual Y position based on selected height (a1-a8)
        // a8 (selectedPosition1Height = 8) = testPos1Y (lowest, no offset)
        // a1 (selectedPosition1Height = 1) = testPos1Y + 7 * spacing (highest)
        float actualPos1Y = testPos1Y + (8 - selectedPosition1Height) * POSITION_HEIGHT_SPACING_INCHES;
        
        // Calculate target absolute positions (negate because positive direction moves toward home switches)
        float targetX = -testPos1X;
        float targetY = -actualPos1Y;
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 1
        motorX->moveInches(moveX);
        moveYWithSpeedAdjustment(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 1;
    }
    
    //! ************************************************************************
    //! STEP 2: EXTEND FORK MOTOR AT POSITION 1
    //! ************************************************************************
    else if (step == 1) {
        // Get current fork motor position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -testPos1Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork motor to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Check if square is present (sensor is active LOW) - only if square sensing is enabled
        // If no square present, retract fork and skip to end of position 4 (step 18)
        extern bool squareSensingEnabled;
        if (squareSensingEnabled && digitalRead(SQUARE_PRESENT_SENSOR_PIN) != LOW) {
            // Retract fork before skipping
            motorFork->moveInches(testPos1Fork);
            
            // Wait for fork motor to finish retracting (check home sensor as safety)
            while (motorFork->isMotorRunning()) {
                if (homeSwitchFork && homeSwitchFork->read()) {
                    motorFork->forceStop();
                    break;
                }
                updateOTA();
                delay(1);
            }
            
            CHECK_PAUSE_AND_CANCEL();
            step = 15;
        } else {
            CHECK_PAUSE_AND_CANCEL();
            step = 2;
        }
    }
    
    //! ************************************************************************
    //! STEP 3: RAISE Y BY 0.5 INCHES AT POSITION 1
    //! ************************************************************************
    else if (step == 2) {
        // Set speed and acceleration for 0.5 inch movement (going up - use reduced speed)
        motorY->setSpeed((long)(TEST_Y_SPEED_FORK_EXTENDED * Y_SPEED_UP_MULTIPLIER));
        motorY->setAcceleration(TEST_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(-0.5);
        
        // Wait for X to finish raising
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        CHECK_PAUSE_AND_CANCEL();
        step = 3;
    }
    
    //! ************************************************************************
    //! STEP 4: RETRACT FORK MOTOR AT POSITION 1
    //! ************************************************************************
    else if (step == 3) {
        motorFork->moveInches(testPos1Fork);
        
        // Wait for fork motor to finish retracting (check home sensor as safety)
        while (motorFork->isMotorRunning()) {
            if (homeSwitchFork && homeSwitchFork->read()) {
                motorFork->forceStop();
                break;
            }
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 4;
    }
    
    //! ************************************************************************
    //! STEP 5: MOVE TO POSITION 2
    //! ************************************************************************
    else if (step == 4) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate target absolute positions (negate because positive direction moves toward home switches)
        float targetX = -testPos2X;
        float targetY = -testPos2Y;
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 2
        motorX->moveInches(moveX);
        moveYWithSpeedAdjustment(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 5;
    }
    
    //! ************************************************************************
    //! STEP 6: EXTEND FORK MOTOR AT POSITION 2
    //! ************************************************************************
    else if (step == 5) {
        // Get current fork motor position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -testPos2Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork motor to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 6;
    }
    
    //! ************************************************************************
    //! STEP 7: LOWER Y BY 0.5 INCHES AT POSITION 2
    //! ************************************************************************
    else if (step == 6) {
        // Turn on suction before Y lowering occurs
        digitalWrite(SUCTION_PIN, HIGH);
        
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(TEST_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(TEST_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(0.5);
        
        // Wait for X to finish lowering
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        CHECK_PAUSE_AND_CANCEL();
        step = 7;
    }
    
    //! ************************************************************************
    //! STEP 8: RETRACT FORK TO POSITION 2
    //! ************************************************************************
    else if (step == 7) {
        motorFork->moveInches(testPos2Fork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        
        // Set step to 8 for when we return from painting state
        step = 8;
        
        // Transition to painting state
        setMachineState(STATE_PAINTING);
        return;
    }
    
    //! ************************************************************************
    //! STEP 9: MOVE TO POSITION 3 (after returning from painting)
    //! ************************************************************************
    else if (step == 8) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate target absolute positions (pos2 but Y is 0.5 lower)
        float targetX = -testPos2X;
        float targetY = -testPos2Y + 0.5;  // 0.5 lower means less negative (add 0.5)
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 3
        motorX->moveInches(moveX);
        moveYWithSpeedAdjustment(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 9;
    }
    
    //! ************************************************************************
    //! STEP 10: EXTEND FORK MOTOR AT POSITION 3
    //! ************************************************************************
    else if (step == 9) {
        // Get current fork motor position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -testPos2Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork motor to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 10;
    }
    
    //! ************************************************************************
    //! STEP 11: MOVE Y UP 0.5 INCHES AT POSITION 3
    //! ************************************************************************
    else if (step == 10) {
        // Set speed and acceleration for 0.5 inch movement (going up - use reduced speed)
        motorY->setSpeed((long)(TEST_Y_SPEED_FORK_EXTENDED * Y_SPEED_UP_MULTIPLIER));
        motorY->setAcceleration(TEST_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(-0.5);  // Negative moves away from home (up)
        
        // Wait for Y to finish moving
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        CHECK_PAUSE_AND_CANCEL();
        step = 11;
    }
    
    //! ************************************************************************
    //! STEP 12: RETRACT FORK MOTOR AT POSITION 3
    //! ************************************************************************
    else if (step == 11) {
        // Get current fork motor position and retract to home
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        motorFork->moveInches(-currentFork);
        
        // Wait for fork motor to finish retracting (check home sensor as safety)
        while (motorFork->isMotorRunning()) {
            if (homeSwitchFork && homeSwitchFork->read()) {
                motorFork->forceStop();
                break;
            }
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 12;
    }
    
    //! ************************************************************************
    //! STEP 13: MOVE TO POSITION 4 (POS1 BUT Y IS 0.5 HIGHER)
    //! ************************************************************************
    else if (step == 12) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate actual Y position based on selected height (a1-a8)
        // a8 (selectedPosition1Height = 8) = testPos1Y (lowest, no offset)
        // a1 (selectedPosition1Height = 1) = testPos1Y + 7 * spacing (highest)
        float actualPos1Y = testPos1Y + (8 - selectedPosition1Height) * POSITION_HEIGHT_SPACING_INCHES;
        
        // Calculate target absolute positions (pos1 but Y is 0.5 higher)
        float targetX = -testPos1X;
        float targetY = -actualPos1Y - 0.5;  // 0.5 higher means more negative (subtract 0.5)
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 4
        motorX->moveInches(moveX);
        moveYWithSpeedAdjustment(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 13;
    }
    
    //! ************************************************************************
    //! STEP 14: EXTEND FORK MOTOR AT POSITION 4
    //! ************************************************************************
    else if (step == 13) {
        // Get current fork motor position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -testPos1Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork motor to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 14;
    }
    
    //! ************************************************************************
    //! STEP 15: LOWER Y BY 0.5 INCHES AT POSITION 4
    //! ************************************************************************
    else if (step == 14) {
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(TEST_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(TEST_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(0.5);
        
        // Wait for Y to finish lowering
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        CHECK_PAUSE_AND_CANCEL();
        step = 15;
    }
    
    //! ************************************************************************
    //! STEP 16: RETRACT FORK MOTOR AT POSITION 4
    //! ************************************************************************
    else if (step == 15) {
        motorFork->moveInches(testPos1Fork);
        
        // Wait for fork motor to finish retracting (check home sensor as safety)
        while (motorFork->isMotorRunning()) {
            if (homeSwitchFork && homeSwitchFork->read()) {
                motorFork->forceStop();
                break;
            }
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 16;
    }
    
    //! ************************************************************************
    //! STEP 17: MOVE X, Y, AND FORK TO POSITION 0 (skip if test all mode cycling)
    //! ************************************************************************
    else if (step == 16) {
        // Skip return to 0 if we're in test all mode and have more heights or columns to test
        if (testAllMode && (currentTestAllHeight < 8 || testAllCurrentColumnIndex < testAllColumnCount - 1)) {
            step = 17;
        } else {
            // Get current positions
            float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
            float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
            float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
            
            // Move X, Y, and Fork to position 0 simultaneously
            motorX->moveInches(-currentX);
            moveYWithSpeedAdjustment(-currentY);
            motorFork->moveInches(-currentFork);
            
            // Wait for all motors to finish (check fork home sensor as safety)
            while (motorX->isMotorRunning() || motorY->isMotorRunning() || motorFork->isMotorRunning()) {
                if (homeSwitchFork && homeSwitchFork->read() && motorFork->isMotorRunning()) {
                    motorFork->forceStop();
                }
                updateOTA();
                delay(1);
            }
            CHECK_PAUSE_AND_CANCEL();
            step = 17;
        }
    }
    
    //! ************************************************************************
    //! STEP 18: HOME X, Y, AND FORK MOTORS
    //! ************************************************************************
    else if (step == 17) {
        // Safety: Ensure paint rotation motor is stopped and disabled
        stopStepper();
        while (isStepperRunning()) {
            delay(10);
        }
        disablePaintRotationMotor();
        
        // Ensure paint gun and suction are off
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        // Ensure servo is at servo home angle (final position)
        if (servo) {
            currentServoAngle = SERVO_HOME_ANGLE;
            servo->write(SERVO_HOME_ANGLE);
        }
        
        // Check if we're in test all mode and need to continue
        if (testAllMode) {
            // Check if we've completed all heights for current column
            if (currentTestAllHeight < 8) {
                // Increment to next height
                currentTestAllHeight++;
                selectedPosition1Height = currentTestAllHeight;
                
                // Reset test state to restart from step 0
                testStarted = false;
                step = 0;
                
                // Return immediately - next loop iteration will restart from step 0
                return;
            } else {
                // All heights completed for current column
                // Check if there are more columns to test
                if (testAllCurrentColumnIndex < testAllColumnCount - 1) {
                    // Home X, Y, and Fork axes before moving to next column (preserve column position)
                    homeAllAxes(false);  // false = don't reset column position
                    
                    // Move to next column
                    testAllCurrentColumnIndex++;
                    
                    // Calculate next column with wrap-around (0-5)
                    int nextColumn = (testAllStartColumn + testAllCurrentColumnIndex) % 6;
                    selectedColumn = nextColumn;
                    
                    // Move storage motor to next column
                    moveToColumn(nextColumn);
                    
                    // Reset height to 1 for new column
                    currentTestAllHeight = 1;
                    selectedPosition1Height = 1;
                    
                    // Reset test state to restart from step 0
                    testStarted = false;
                    step = 0;
                    
                    // Return immediately - next loop iteration will restart from step 0
                    return;
                } else {
                    // All columns completed - proceed to cleanup
                    // Home all axes (X, Y, and Fork) but preserve column position
                    homeAllAxes(false);  // false = don't reset column position
                    
                    // Reset test all mode
                    testAllMode = false;
                    currentTestAllHeight = 1;
                    testAllColumnCount = 1;
                    testAllStartColumn = 0;
                    testAllCurrentColumnIndex = 0;
                    
                    // Return to idle
                    testStarted = false;
                    setMachineState(STATE_IDLE);
                }
            }
        } else {
            // Single test complete - now do cleanup (homing only happens at the end)
            // Home all axes (X, Y, and Fork) but preserve column position
            homeAllAxes(false);  // false = don't reset column position
            
            // Return to idle
            testStarted = false;
            setMachineState(STATE_IDLE);
        }
    }
}
