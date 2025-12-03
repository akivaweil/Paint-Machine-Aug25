#include <Arduino.h>
#include "StateMachine/STATES/02_TEST.h"
#include "../../config/Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
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

// Parallel sequence handler (called during wait loops)
// Paint motor 360 turn starts immediately after pos2, servo returns to 135 at 180-degree mark
static long paintMotorTargetPosition = 0;
static long paintMotorHalfwayPosition = 0;
static bool servoReturnStarted = false;

void updateParallelSequence(bool& parallelSequenceStarted, int& parallelStep) {
    if (!parallelSequenceStarted) return;
    
    if (parallelStep == 0) {
        // Turn on paint gun and move servo to 220 (paint motor already running from pos2)
        digitalWrite(PAINT_GUN_PIN, HIGH);
        servoReturnStarted = false;
        parallelStep = 1;
    }
    else if (parallelStep == 1) {
        // Rotate servo to 220 degrees (non-blocking) while paint motor runs
        updateServoNonBlocking(220.0);
        
        // Check if paint motor reached 180 degrees (halfway) - trigger servo return
        if (!servoReturnStarted && motorPaintRotation) {
            long currentPos = motorPaintRotation->getCurrentPosition();
            // Check if we've passed the halfway point
            if ((paintMotorTargetPosition > paintMotorHalfwayPosition && currentPos >= paintMotorHalfwayPosition) ||
                (paintMotorTargetPosition < paintMotorHalfwayPosition && currentPos <= paintMotorHalfwayPosition)) {
                servoReturnStarted = true;
                parallelStep = 2;
            }
        }
        
        // Also check if motor already finished (in case it's fast)
        if (!servoReturnStarted && motorPaintRotation && !motorPaintRotation->isMotorRunning()) {
            servoReturnStarted = true;
            parallelStep = 2;
        }
    }
    else if (parallelStep == 2) {
        // Rotate servo back to 135 degrees while paint motor finishes 360
        updateServoNonBlocking(135.0);
        
        bool servoComplete = abs(currentServoAngle - 135.0) < 0.5;
        bool motorComplete = !motorPaintRotation || !motorPaintRotation->isMotorRunning();
        
        if (servoComplete && motorComplete) {
            parallelStep = 3;
        }
    }
    else if (parallelStep == 3) {
        // Paint motor finished - disable it
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
            delay(50);
        }
        disablePaintRotationMotor();
        
        // Turn off paint gun
        digitalWrite(PAINT_GUN_PIN, LOW);
        
        // Parallel sequence complete
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
        
        // Move servo to 135 degrees at the beginning
        if (servo) {
            currentServoAngle = 135.0;
            servo->write(135.0);
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
        
        // Calculate target absolute positions (negate because positive direction moves toward home switches)
        float targetX = -testPos1X;
        float targetY = -testPos1Y;
        
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
        step = 2;
    }
    
    //! ************************************************************************
    //! STEP 3: RAISE Y BY 0.5 INCHES AT POSITION 1
    //! ************************************************************************
    else if (step == 2) {
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(5000);
        motorY->setAcceleration(3000);
        
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
        motorY->setSpeed(5000);
        motorY->setAcceleration(3000);
        
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
            long startPos = motorPaintRotation->getCurrentPosition();
            long stepsToMove = PAINT_ROTATION_MOTOR_STEPS_PER_REV_OUTPUT;  // 38400 steps = 360 degrees
            paintMotorHalfwayPosition = startPos + (stepsToMove / 2);  // 180 degree mark
            paintMotorTargetPosition = startPos + stepsToMove;          // 360 degree mark
            motorPaintRotation->moveSteps(stepsToMove);
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
        
        // Ensure paint gun is off
        digitalWrite(PAINT_GUN_PIN, LOW);
        
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
        step = 12;
    }
    
    //! ************************************************************************
    //! STEP 13: MOVE Y UP 0.5 INCHES AT POSITION 3
    //! ************************************************************************
    else if (step == 12) {
        // Set speed and acceleration for 0.5 inch movement
        motorY->setSpeed(5000);
        motorY->setAcceleration(3000);
        
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
        
        // Calculate target absolute positions (pos1 but Y is 0.5 higher)
        float targetX = -testPos1X;
        float targetY = -testPos1Y - 0.5;  // 0.5 higher means more negative (subtract 0.5)
        
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
        motorY->setSpeed(5000);
        motorY->setAcceleration(3000);
        
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
    //! STEP 19: RETURN X TO HOME POSITION
    //! ************************************************************************
    else if (step == 18) {
        // Get current X position
        float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
        
        // Move X back to home (0)
        motorX->moveInches(-currentX);
        
        // Wait for X motor to finish
        while (motorX->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        step = 19;
    }
    
    //! ************************************************************************
    //! STEP 20: RETURN Y AND FORK MOTOR TO HOME POSITION
    //! ************************************************************************
    else if (step == 19) {
        // Get current positions
        float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
        float currentFork = motorFork->stepsToInches(motorFork->getCurrentPosition());
        
        // Move Y and fork motor back to home (0, 0)
        motorY->moveInches(-currentY);
        motorFork->moveInches(-currentFork);
        
        // Wait for Y and fork motors to finish
        while (motorY->isMotorRunning() || motorFork->isMotorRunning()) {
            updateOTA();
            delay(1);
        }
        
        // Safety: Ensure paint rotation motor is stopped and disabled
        if (motorPaintRotation) {
            motorPaintRotation->stopContinuous();
            while (motorPaintRotation->isMotorRunning()) {
                delay(10);
            }
            delay(50);
        }
        disablePaintRotationMotor();
        
        // Ensure paint gun is off
        digitalWrite(PAINT_GUN_PIN, LOW);
        
        // Ensure servo is at 135 degrees (final position)
        if (servo) {
            updateServoNonBlocking(135.0);
            // Wait for servo to reach final position
            while (abs(currentServoAngle - 135.0) > 0.5) {
                updateServoNonBlocking(135.0);
                delay(10);
            }
        }
        
        // Test complete - return to idle
        testStarted = false;
        setMachineState(STATE_IDLE);
    }
}

