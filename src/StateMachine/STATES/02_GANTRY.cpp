#include <Arduino.h>
#include "StateMachine/STATES/02_GANTRY.h"
#include "../../config/Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "Web_Manager.h"
#include "ServoControl.h"
#include "../../config/Pin_Definitions.h"

// External motor instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;
extern StepperMotor* motorPaintRotation;

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

// Position values (set from web interface)
extern float pos1X;
extern float pos1Y;
extern float pos1Fork;
extern float pos2X;
extern float pos2Y;
extern float pos2Fork;
extern int selectedPosition1Height;  // Position 1 height selection (1-8, where 8 = a8/lowest)
extern int selectedColumn;  // Selected column for cycle (0-5, where 0=A, 5=F)
extern bool cycleAllMode;  // Flag to track if we're in "cycle all" mode
extern int currentCycleAllHeight;  // Track which height we're currently cycling (1-8)
extern int cycleAllColumnCount;  // Number of columns to cycle (1-6)
extern int cycleAllStartColumn;  // Starting column position (from physical position)
extern int cycleAllCurrentColumnIndex;  // Current column index in the cycle sequence (0 to cycleAllColumnCount-1)

// Paint gun pin
#include "../../config/Pin_Definitions.h"

//* ************************************************************************
//* ************************ GANTRY STATE *********************************
//* ************************************************************************

void gantryState() {
    static int step = 0;
    static bool cycleStarted = false;
    
    // Check for cancel at start of function
    if (cycleCancelled) {
        // Cleanup: stop all motors immediately
        if (motorX) motorX->forceStop();
        if (motorY) motorY->forceStop();
        if (motorFork) motorFork->forceStop();
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
            disablePaintRotationMotor();
        }
        
        // Turn off paint gun and suction
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        // Home all motors (preserve column position)
        homeAllAxes(false);
        
        // Reset flags and state
        cycleCancelled = false;
        cyclePaused = false;
        cycleStarted = false;
        step = 0;
        
        // Return to idle
        setMachineState(STATE_IDLE);
        return;
    }
    
    // Initialize on first entry
    if (!cycleStarted) {
        step = 0;
        cycleStarted = true;
        cyclePaused = false;  // Reset pause flag on new cycle
        cycleCancelled = false;  // Reset cancel flag on new cycle
        
        // Apply motor settings from dashboard before starting cycle
        applyMotorSettings();
        
        // Turn on pressure pot automatically when cycle starts
        digitalWrite(PRESSURE_POT_PIN, HIGH);
        extern bool pressurePotState;
        pressurePotState = true;
        
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
        // a8 (selectedPosition1Height = 8) = pos1Y (lowest, no offset)
        // a1 (selectedPosition1Height = 1) = pos1Y + 7 * spacing (highest)
        float actualPos1Y = pos1Y + (8 - selectedPosition1Height) * POSITION_HEIGHT_SPACING_INCHES;
        
        // Calculate target absolute positions (negate because positive direction moves toward home switches)
        float targetX = -pos1X;
        float targetY = -actualPos1Y;
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 1
        motorX->moveInches(moveX);
        motorY->moveInches(moveY);
        
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
        float targetFork = -pos1Fork;
        
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
            motorFork->moveInches(pos1Fork);
            
            // Wait for fork motor to finish retracting
            while (motorFork->isMotorRunning()) {
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
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(CYCLE_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(CYCLE_Y_ACCEL_FORK_EXTENDED);
        
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
        motorFork->moveInches(pos1Fork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
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
        float targetX = -pos2X;
        float targetY = -pos2Y;
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 2
        motorX->moveInches(moveX);
        motorY->moveInches(moveY);
        
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
        float targetFork = -pos2Fork;
        
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
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(CYCLE_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(CYCLE_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(0.5);
        
        // Wait for X to finish lowering
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        CHECK_PAUSE_AND_CANCEL();
        
        // Set step to 7 for when we return from painting state
        step = 7;
        
        // Transition to painting state
        setMachineState(STATE_PAINTING);
        return;
    }
    
    //! ************************************************************************
    //! STEP 8: MOVE TO POSITION 3 (after returning from painting)
    //! ************************************************************************
    else if (step == 7) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate target absolute positions (pos2 but Y is 0.5 lower)
        float targetX = -pos2X;
        float targetY = -pos2Y + 0.5;  // 0.5 lower means less negative (add 0.5)
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 3
        motorX->moveInches(moveX);
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 8;
    }
    
    //! ************************************************************************
    //! STEP 9: EXTEND FORK MOTOR AT POSITION 3
    //! ************************************************************************
    else if (step == 8) {
        // Get current fork motor position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -pos2Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork motor to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 9;
    }
    
    //! ************************************************************************
    //! STEP 10: MOVE Y UP 0.5 INCHES AT POSITION 3
    //! ************************************************************************
    else if (step == 9) {
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(CYCLE_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(CYCLE_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(-0.5);  // Negative moves away from home (up)
        
        // Wait for Y to finish moving
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        CHECK_PAUSE_AND_CANCEL();
        step = 10;
    }
    
    //! ************************************************************************
    //! STEP 11: RETRACT FORK MOTOR AT POSITION 3
    //! ************************************************************************
    else if (step == 10) {
        // Get current fork motor position and retract to home
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        motorFork->moveInches(-currentFork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 11;
    }
    
    //! ************************************************************************
    //! STEP 12: MOVE TO POSITION 4 (POS1 BUT Y IS 0.5 HIGHER)
    //! ************************************************************************
    else if (step == 11) {
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate actual Y position based on selected height (a1-a8)
        // a8 (selectedPosition1Height = 8) = pos1Y (lowest, no offset)
        // a1 (selectedPosition1Height = 1) = pos1Y + 7 * spacing (highest)
        float actualPos1Y = pos1Y + (8 - selectedPosition1Height) * POSITION_HEIGHT_SPACING_INCHES;
        
        // Calculate target absolute positions (pos1 but Y is 0.5 higher)
        float targetX = -pos1X;
        float targetY = -actualPos1Y - 0.5;  // 0.5 higher means more negative (subtract 0.5)
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to absolute position 4
        motorX->moveInches(moveX);
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 12;
    }
    
    //! ************************************************************************
    //! STEP 13: EXTEND FORK MOTOR AT POSITION 4
    //! ************************************************************************
    else if (step == 12) {
        // Get current fork motor position
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Calculate target absolute position (negate because positive direction moves toward home switches)
        float targetFork = -pos1Fork;
        
        // Calculate relative movement needed to reach absolute position
        float moveFork = targetFork - currentFork;
        
        motorFork->moveInches(moveFork);
        
        // Wait for fork motor to finish extending
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 13;
    }
    
    //! ************************************************************************
    //! STEP 14: LOWER Y BY 0.5 INCHES AT POSITION 4
    //! ************************************************************************
    else if (step == 13) {
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(CYCLE_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(CYCLE_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(0.5);
        
        // Wait for Y to finish lowering
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        CHECK_PAUSE_AND_CANCEL();
        step = 14;
    }
    
    //! ************************************************************************
    //! STEP 15: RETRACT FORK MOTOR AT POSITION 4
    //! ************************************************************************
    else if (step == 14) {
        motorFork->moveInches(pos1Fork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        CHECK_PAUSE_AND_CANCEL();
        step = 15;
    }
    
    //! ************************************************************************
    //! STEP 16: MOVE X, Y, AND FORK TO POSITION 0 (skip if cycle all mode cycling)
    //! ************************************************************************
    else if (step == 15) {
        // Skip return to 0 if we're in cycle all mode and have more heights or columns to cycle
        if (cycleAllMode && (currentCycleAllHeight < 8 || cycleAllCurrentColumnIndex < cycleAllColumnCount - 1)) {
            step = 16;
        } else {
            // Get current positions
            float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
            float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
            float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
            
            // Move X, Y, and Fork to position 0 simultaneously
            motorX->moveInches(-currentX);
            motorY->moveInches(-currentY);
            motorFork->moveInches(-currentFork);
            
            // Wait for all motors to finish
            while (motorX->isMotorRunning() || motorY->isMotorRunning() || motorFork->isMotorRunning()) {
                updateOTA();
                delay(1);
            }
            CHECK_PAUSE_AND_CANCEL();
            step = 16;
        }
    }
    
    //! ************************************************************************
    //! STEP 17: HOME X, Y, AND FORK MOTORS
    //! ************************************************************************
    else if (step == 16) {
        // Safety: Ensure paint rotation motor is stopped and disabled
        if (motorPaintRotation) {
            motorPaintRotation->stopContinuous();
            while (motorPaintRotation->isMotorRunning()) {
                delay(10);
            }
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
        
        // Check if we're in cycle all mode and need to continue
        if (cycleAllMode) {
            // Check if we've completed all heights for current column
            if (currentCycleAllHeight < 8) {
                // Increment to next height
                currentCycleAllHeight++;
                selectedPosition1Height = currentCycleAllHeight;
                
                // Reset cycle state to restart from step 0
                cycleStarted = false;
                step = 0;
                
                // Return immediately - next loop iteration will restart from step 0
                return;
            } else {
                // All heights completed for current column
                // Check if there are more columns to cycle
                if (cycleAllCurrentColumnIndex < cycleAllColumnCount - 1) {
                    // Home X, Y, and Fork axes before moving to next column (preserve column position)
                    homeAllAxes(false);  // false = don't reset column position
                    
                    // Move to next column
                    cycleAllCurrentColumnIndex++;
                    
                    // Calculate next column with wrap-around (0-5)
                    int nextColumn = (cycleAllStartColumn + cycleAllCurrentColumnIndex) % 6;
                    selectedColumn = nextColumn;
                    
                    // Move storage motor to next column
                    moveToColumn(nextColumn);
                    
                    // Reset height to 1 for new column
                    currentCycleAllHeight = 1;
                    selectedPosition1Height = 1;
                    
                    // Reset cycle state to restart from step 0
                    cycleStarted = false;
                    step = 0;
                    
                    // Return immediately - next loop iteration will restart from step 0
                    return;
                } else {
                    // All columns completed - proceed to cleanup
                    // Home all axes (X, Y, and Fork) but preserve column position
                    homeAllAxes(false);  // false = don't reset column position
                    
                    // Reset cycle all mode
                    cycleAllMode = false;
                    currentCycleAllHeight = 1;
                    cycleAllColumnCount = 1;
                    cycleAllStartColumn = 0;
                    cycleAllCurrentColumnIndex = 0;
                    
                    // Return to idle
                    cycleStarted = false;
                    setMachineState(STATE_IDLE);
                }
            }
        } else {
            // Single cycle complete - now do cleanup (homing only happens at the end)
            // Home all axes (X, Y, and Fork) but preserve column position
            homeAllAxes(false);  // false = don't reset column position
            
            // Return to idle
            cycleStarted = false;
            setMachineState(STATE_IDLE);
        }
    }
}

