#include <Arduino.h>
#include "StateMachine/STATES/02_TEST.h"
#include "../../config/Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "Web_Manager.h"
#include "ServoControl.h"

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

// Test position values (set from web interface)
extern float testPos1X;
extern float testPos1Y;
extern float testPos1Fork;
extern float testPos2X;
extern float testPos2Y;
extern float testPos2Fork;
extern int selectedPosition1Height;  // Position 1 height selection (1-8, where 8 = a8/lowest)
extern bool testAllMode;  // Flag to track if we're in "test all" mode
extern int currentTestAllHeight;  // Track which height we're currently testing (1-8)

// Paint gun pin
#include "../../config/Pin_Definitions.h"

//* ************************************************************************
//* ************************ TEST STATE ***********************************
//* ************************************************************************

// Non-blocking servo movement helper
void updateServoNonBlocking(float targetAngle) {
    if (!servo) return;
    
    const float stepSize = 0.5;  // Step size in degrees
    const float stepDelayMs = (stepSize / servoSpeed) * 1000.0;  // Delay in milliseconds
    static unsigned long lastServoUpdate = 0;
    
    unsigned long now = millis();
    if (now - lastServoUpdate >= (unsigned long)stepDelayMs) {
        float diff = targetAngle - currentServoAngle;
        
        if (abs(diff) > stepSize) {
            float increment = (diff > 0) ? stepSize : -stepSize;
            currentServoAngle += increment;
            servo->write(currentServoAngle);
        } else {
            currentServoAngle = targetAngle;
            servo->write(currentServoAngle);
        }
        
        lastServoUpdate = now;
    }
}

// Paint motor 360 turn tracking
static long paintMotor360StepsTarget = 0;
static long paintMotor360StepsStart = 0;

// Parallel sequence handler (called during wait loops)
// Handles servo movement and paint gun, paint motor runs independently
void updateParallelSequence(bool& parallelSequenceStarted, int& parallelStep) {
    if (!parallelSequenceStarted) return;
    
    if (parallelStep == 0) {
        // Turn on paint gun and start servo to 220
        digitalWrite(PAINT_GUN_PIN, HIGH);
        parallelStep = 1;
    }
    else if (parallelStep == 1) {
        // Ensure suction is on while painting motor is rotating
        if (motorPaintRotation && motorPaintRotation->isMotorRunning()) {
            digitalWrite(SUCTION_PIN, HIGH);
        }
        
        // Rotate servo to 220 degrees (non-blocking)
        updateServoNonBlocking(220.0);
        
        // Check if paint motor reached 180 degrees (halfway through 360)
        if (motorPaintRotation) {
            long stepsCompleted = motorPaintRotation->getCurrentPosition() - paintMotor360StepsStart;
            long halfwaySteps = paintRotationMotorStepsPerRevOutput / 2;
            
            if (stepsCompleted >= halfwaySteps) {
                parallelStep = 2;  // Start returning servo
            }
        }
        
        // Fallback: if motor stops before halfway, still continue
        if (motorPaintRotation && !motorPaintRotation->isMotorRunning()) {
            parallelStep = 2;
        }
    }
    else if (parallelStep == 2) {
        // Ensure suction is on while painting motor is rotating
        if (motorPaintRotation && motorPaintRotation->isMotorRunning()) {
            digitalWrite(SUCTION_PIN, HIGH);
        }
        
        // Rotate servo back to servo home angle
        updateServoNonBlocking(SERVO_HOME_ANGLE);
        
        bool servoComplete = abs(currentServoAngle - SERVO_HOME_ANGLE) < 0.5;
        bool motorComplete = !motorPaintRotation || !motorPaintRotation->isMotorRunning();
        
        if (servoComplete && motorComplete) {
            parallelStep = 3;
        }
    }
    else if (parallelStep == 3) {
        // Cleanup: disable motor and turn off paint gun and suction
        disablePaintRotationMotor();
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        parallelSequenceStarted = false;
    }
}

void testState() {
    static int step = 0;
    static bool testStarted = false;
    static bool parallelSequenceStarted = false;
    static int parallelStep = 0;
    
    // Initialize on first entry
    if (!testStarted) {
        step = 0;
        testStarted = true;
        parallelSequenceStarted = false;
        parallelStep = 0;
        
        // Apply motor settings from dashboard before starting test
        applyMotorSettings();
        
        // Move servo to servo home angle at the beginning
        if (servo) {
            currentServoAngle = SERVO_HOME_ANGLE;
            servo->write(SERVO_HOME_ANGLE);
        }
    }
    
    // Handle parallel sequence (runs independently)
    updateParallelSequence(parallelSequenceStarted, parallelStep);
    
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
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
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
        
        // Check if square is present (sensor is active LOW)
        // If no square present, retract fork and skip to end of position 4 (step 18)
        if (digitalRead(SQUARE_PRESENT_SENSOR_PIN) != LOW) {
            // Retract fork before skipping
            motorFork->moveInches(testPos1Fork);
            
            // Wait for fork motor to finish retracting
            while (motorFork->isMotorRunning()) {
                updateOTA();
                delay(1);
            }
            
            step = 18;
        } else {
            step = 2;
        }
    }
    
    //! ************************************************************************
    //! STEP 3: RAISE Y BY 0.5 INCHES AT POSITION 1
    //! ************************************************************************
    else if (step == 2) {
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(TEST_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(TEST_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(-0.5);
        
        // Wait for X to finish raising
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        step = 3;
    }
    
    //! ************************************************************************
    //! STEP 4: RETRACT FORK MOTOR AT POSITION 1
    //! ************************************************************************
    else if (step == 3) {
        motorFork->moveInches(testPos1Fork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
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
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
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
        step = 6;
    }
    
    //! ************************************************************************
    //! STEP 7: LOWER Y BY 0.5 INCHES AT POSITION 2
    //! ************************************************************************
    else if (step == 6) {
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
        
        step = 7;
    }
    
    //! ************************************************************************
    //! STEP 8: RETRACT FORK MOTOR AT POSITION 2, START PAINT MOTOR 360 TURN
    //! ************************************************************************
    else if (step == 7) {
        motorFork->moveInches(testPos2Fork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Start paint motor 360 turn immediately after pos2
        enablePaintRotationMotor();
        if (motorPaintRotation) {
            paintMotor360StepsStart = motorPaintRotation->getCurrentPosition();
            paintMotor360StepsTarget = paintRotationMotorStepsPerRevOutput;
            motorPaintRotation->moveSteps(paintMotor360StepsTarget);
            // Turn on suction when painting motor starts rotating
            digitalWrite(SUCTION_PIN, HIGH);
        }
        
        step = 8;
    }
    
    //! ************************************************************************
    //! STEP 9: MOVE TO WAITING POSITION (5 INCHES RIGHT OF POSITION 3)
    //! ************************************************************************
    else if (step == 8) {
        // Start parallel sequence when beginning to move to waiting position
        if (!parallelSequenceStarted) {
            parallelSequenceStarted = true;
            parallelStep = 0;
        }
        
        // Get current positions
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        
        // Calculate target absolute positions (5 inches right of pos3, same Y as pos3)
        // Position 3 is: X = -testPos2X, Y = -testPos2Y + 0.5
        // Waiting position is: X = -testPos2X + 5, Y = -testPos2Y + 0.5
        float targetX = -testPos2X + 5.0;  // 5 inches right (more positive)
        float targetY = -testPos2Y + 0.5;   // Same Y as pos3
        
        // Calculate relative movement needed to reach absolute position
        float moveX = targetX - currentX;
        float moveY = targetY - currentY;
        
        // Move X and Y simultaneously to waiting position
        motorX->moveInches(moveX);
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish (parallel sequence continues running)
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateParallelSequence(parallelSequenceStarted, parallelStep);
            updateOTA();
            delay(1);
        }
        step = 9;
    }
    
    //! ************************************************************************
    //! STEP 10: WAIT FOR PARALLEL SEQUENCE (SERVO AND PAINTING MOTOR) TO COMPLETE
    //! ************************************************************************
    else if (step == 9) {
        // Wait for parallel sequence to complete
        while (parallelSequenceStarted) {
            updateParallelSequence(parallelSequenceStarted, parallelStep);
            updateOTA();
            delay(1);
        }
        
        // Safety: Ensure paint rotation motor is stopped and disabled
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
            delay(50);
        }
        disablePaintRotationMotor();
        
        // Ensure paint gun and suction are off
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        step = 10;
    }
    
    //! ************************************************************************
    //! STEP 11: MOVE TO POSITION 3
    //! ************************************************************************
    else if (step == 10) {
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
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 11;
    }
    
    //! ************************************************************************
    //! STEP 12: EXTEND FORK MOTOR AT POSITION 3
    //! ************************************************************************
    else if (step == 11) {
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
        
        // Check if square is present (sensor is active LOW)
        // If no square present, retract fork and skip to end of position 4 (step 18)
        if (digitalRead(SQUARE_PRESENT_SENSOR_PIN) != LOW) {
            // Retract fork to home before skipping
            float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
            motorFork->moveInches(-currentFork);
            
            // Wait for fork motor to finish retracting
            while (motorFork->isMotorRunning()) {
                updateOTA();
                delay(1);
            }
            
            step = 18;
        } else {
            step = 12;
        }
    }
    
    //! ************************************************************************
    //! STEP 13: MOVE Y UP 0.5 INCHES AT POSITION 3
    //! ************************************************************************
    else if (step == 12) {
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(TEST_Y_SPEED_FORK_EXTENDED);
        motorY->setAcceleration(TEST_Y_ACCEL_FORK_EXTENDED);
        
        motorY->moveInches(-0.5);  // Negative moves away from home (up)
        
        // Wait for Y to finish moving
        while (motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Restore motor settings
        applyMotorSettings();
        
        step = 13;
    }
    
    //! ************************************************************************
    //! STEP 14: RETRACT FORK MOTOR AT POSITION 3
    //! ************************************************************************
    else if (step == 13) {
        // Get current fork motor position and retract to home
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        motorFork->moveInches(-currentFork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 14;
    }
    
    //! ************************************************************************
    //! STEP 15: MOVE TO POSITION 4 (POS1 BUT Y IS 0.5 HIGHER)
    //! ************************************************************************
    else if (step == 14) {
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
        motorY->moveInches(moveY);
        
        // Wait for both motors to finish
        while (motorX->isMotorRunning() || motorY->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 15;
    }
    
    //! ************************************************************************
    //! STEP 16: EXTEND FORK MOTOR AT POSITION 4
    //! ************************************************************************
    else if (step == 15) {
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
        step = 16;
    }
    
    //! ************************************************************************
    //! STEP 17: LOWER Y BY 0.5 INCHES AT POSITION 4
    //! ************************************************************************
    else if (step == 16) {
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
        
        step = 17;
    }
    
    //! ************************************************************************
    //! STEP 18: RETRACT FORK MOTOR AT POSITION 4
    //! ************************************************************************
    else if (step == 17) {
        motorFork->moveInches(testPos1Fork);
        
        // Wait for fork motor to finish retracting
        while (motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 18;
    }
    
    //! ************************************************************************
    //! STEP 19: MOVE X, Y, AND FORK TO POSITION 0 (skip if test all mode cycling)
    //! ************************************************************************
    else if (step == 18) {
        // Skip return to 0 if we're in test all mode and have more heights to test
        if (testAllMode && currentTestAllHeight < 8) {
            step = 19;
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
            step = 19;
        }
    }
    
    //! ************************************************************************
    //! STEP 20: HOME X, Y, AND FORK MOTORS
    //! ************************************************************************
    else if (step == 19) {
        // Safety: Ensure paint rotation motor is stopped and disabled
        if (motorPaintRotation) {
            motorPaintRotation->stopContinuous();
            while (motorPaintRotation->isMotorRunning()) {
                delay(10);
            }
            delay(50);
        }
        disablePaintRotationMotor();
        
        // Ensure paint gun and suction are off
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        // Ensure servo is at servo home angle (final position)
        if (servo) {
            updateServoNonBlocking(SERVO_HOME_ANGLE);
            // Wait for servo to reach final position
            while (abs(currentServoAngle - SERVO_HOME_ANGLE) > 0.5) {
                updateServoNonBlocking(SERVO_HOME_ANGLE);
                delay(10);
            }
        }
        
        // Check if we're in test all mode and need to continue
        if (testAllMode && currentTestAllHeight < 8) {
            // Increment to next height
            currentTestAllHeight++;
            selectedPosition1Height = currentTestAllHeight;
            
            // Reset test state to restart from step 0
            testStarted = false;
            step = 0;
            parallelSequenceStarted = false;
            parallelStep = 0;
            
            // Return immediately - next loop iteration will restart from step 0
            return;
        } else {
            // All tests complete - now do cleanup (homing only happens at the end)
            // Home all axes (X, Y, and Fork)
            homeAllAxes();
            
            // Reset test all mode if it was active
            if (testAllMode) {
                testAllMode = false;
                currentTestAllHeight = 1;
            }
            
            // Return to idle
            testStarted = false;
            setMachineState(STATE_IDLE);
        }
    }
}

