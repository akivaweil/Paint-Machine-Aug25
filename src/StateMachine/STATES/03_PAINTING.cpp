#include <Arduino.h>
#include "StateMachine/STATES/03_PAINTING.h"
#include "../../config/Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
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
#define STATE_TEST 2

// Cycle control flags
extern bool cyclePaused;
extern bool cycleCancelled;

// Test position values (set from web interface)
extern float testPos2X;
extern float testPos2Y;
extern float testPos2Fork;

// Paint rotation motor steps per revolution
extern long paintRotationMotorStepsPerRevOutput;

// Macro to check for pause after step completion
#define CHECK_PAUSE_AND_CANCEL() \
    do { \
        while (cyclePaused && !cycleCancelled) { \
            updateOTA(); \
            updateParallelSequence(parallelSequenceStarted, parallelStep); \
            delay(10); \
        } \
        if (cycleCancelled) return; \
    } while(0)

//* ************************************************************************
//* ************************ PAINTING STATE ********************************
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

void paintingState() {
    static int step = 0;
    static bool paintingStarted = false;
    static bool parallelSequenceStarted = false;
    static int parallelStep = 0;
    
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
        
        // Reset flags and state
        cycleCancelled = false;
        cyclePaused = false;
        paintingStarted = false;
        step = 0;
        parallelSequenceStarted = false;
        parallelStep = 0;
        
        // Return to test state (test state will handle further cleanup)
        setMachineState(STATE_TEST);
        return;
    }
    
    // Initialize on first entry
    if (!paintingStarted) {
        step = 0;
        paintingStarted = true;
        parallelSequenceStarted = false;
        parallelStep = 0;
        cyclePaused = false;  // Reset pause flag on new painting
        cycleCancelled = false;  // Reset cancel flag on new painting
    }
    
    // Handle parallel sequence (runs independently)
    updateParallelSequence(parallelSequenceStarted, parallelStep);
    
    //! ************************************************************************
    //! STEP 0: RETRACT FORK MOTOR AT POSITION 2, START PAINT MOTOR 360 TURN
    //! ************************************************************************
    if (step == 0) {
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
        
        CHECK_PAUSE_AND_CANCEL();
        step = 1;
    }
    
    //! ************************************************************************
    //! STEP 1: MOVE TO WAITING POSITION (5 INCHES RIGHT OF POSITION 3)
    //! ************************************************************************
    else if (step == 1) {
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
        CHECK_PAUSE_AND_CANCEL();
        step = 2;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR PARALLEL SEQUENCE (SERVO AND PAINTING MOTOR) TO COMPLETE
    //! ************************************************************************
    else if (step == 2) {
        // Wait for parallel sequence to complete
        while (parallelSequenceStarted) {
            updateParallelSequence(parallelSequenceStarted, parallelStep);
            updateOTA();
            delay(1);
        }
        
        // Safety: Ensure paint rotation motor is stopped and disabled
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
        }
        disablePaintRotationMotor();
        
        // Ensure paint gun and suction are off
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        CHECK_PAUSE_AND_CANCEL();
        
        // Reset painting state flags
        paintingStarted = false;
        step = 0;
        parallelSequenceStarted = false;
        parallelStep = 0;
        
        // Return to test state to continue with remaining steps
        setMachineState(STATE_TEST);
    }
}

//* ************************************************************************
//* ************************ TEST PAINT CYCLE ******************************
//* ************************************************************************

void testPaintCycle() {
    static bool testCycleStarted = false;
    static bool parallelSequenceStarted = false;
    static int parallelStep = 0;
    static long paintMotor360StepsStart = 0;
    
    // Initialize on first entry
    if (!testCycleStarted) {
        testCycleStarted = true;
        parallelSequenceStarted = false;
        parallelStep = 0;
    }
    
    // Handle parallel sequence (servo and paint motor)
    if (!parallelSequenceStarted) {
        parallelSequenceStarted = true;
        parallelStep = 0;
    }
    
    // Turn on paint gun and start servo to 220
    if (parallelStep == 0) {
        digitalWrite(PAINT_GUN_PIN, HIGH);
        
        // Start paint motor 360 turn
        enablePaintRotationMotor();
        if (motorPaintRotation) {
            paintMotor360StepsStart = motorPaintRotation->getCurrentPosition();
            long paintMotor360StepsTarget = paintRotationMotorStepsPerRevOutput;
            motorPaintRotation->moveSteps(paintMotor360StepsTarget);
            // Turn on suction when painting motor starts rotating
            digitalWrite(SUCTION_PIN, HIGH);
        }
        
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
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
        }
        disablePaintRotationMotor();
        digitalWrite(PAINT_GUN_PIN, LOW);
        digitalWrite(SUCTION_PIN, LOW);
        
        // Reset flags
        testCycleStarted = false;
        parallelSequenceStarted = false;
        parallelStep = 0;
        
        // Return to IDLE state
        extern void setMachineState(int state);
        #define STATE_IDLE 0
        setMachineState(STATE_IDLE);
        return;
    }
    
    // Update OTA during operation
    updateOTA();
    delay(1);
}

