#include <Arduino.h>
#include "StateMachine/STATES/03_PAINTING.h"
#include "../../config/Config.h"
#include "../../config/Painting_Config.h"
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
extern void setServoAngle(float angle);

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_GANTRY 2

// Cycle control flags
extern bool cyclePaused;
extern bool cycleCancelled;

// Test position values (set from web interface)
extern float testPos2X;
extern float testPos2Y;
extern float testPos2Fork;

// Paint rotation motor steps per revolution
extern long paintRotationMotorStepsPerRevOutput;

// Test mode flag
extern bool testModeEnabled;

//* ************************************************************************
//* ************************ PAINTING STATE ********************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ HELPER FUNCTIONS                                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Forward declarations
void updateServoMovement();
bool updateServoPositionByRotation(long startPosition, bool resetFlags);

// Wait for a motor to finish moving
void waitForMotor(StepperMotor* motor) {
    while (motor && motor->isMotorRunning()) {
        updateServoMovement();  // Update servo while waiting
        updateOTA();
        if (cycleCancelled) return;
        delay(1);
    }
}

// Wait for multiple motors to finish moving
void waitForMotors(StepperMotor* motor1, StepperMotor* motor2) {
    while ((motor1 && motor1->isMotorRunning()) || (motor2 && motor2->isMotorRunning())) {
        updateServoMovement();  // Update servo while waiting
        updateOTA();
        if (cycleCancelled) return;
        delay(1);
    }
}

// Wait for paint rotation motor to complete X revolutions from a start position
void waitForPaintRotationRevolutions(long startPosition, float revolutions) {
    if (!motorPaintRotation) return;
    
    long targetSteps = paintRotationMotorStepsPerRevOutput * revolutions;
    
    while (motorPaintRotation->isMotorRunning()) {
        long currentSteps = motorPaintRotation->getCurrentPosition() - startPosition;
        if (currentSteps >= targetSteps) {
            break;
        }
        updateServoMovement();  // Update servo while waiting
        updateServoPositionByRotation(startPosition, false);  // Update servo position based on rotation
        updateOTA();
        if (cycleCancelled) return;
        delay(1);
    }
}

// Turn on paint gun
void turnOnPaintGun() {
    digitalWrite(PAINT_GUN_PIN, HIGH);
}

// Turn off paint gun
void turnOffPaintGun() {
    digitalWrite(PAINT_GUN_PIN, LOW);
}

// Turn on suction
void turnOnSuction() {
    digitalWrite(SUCTION_PIN, HIGH);
}

// Turn off suction
void turnOffSuction() {
    digitalWrite(SUCTION_PIN, LOW);
}

// Retract fork to position 2
void retractForkToPosition2() {
    if (motorFork) {
        motorFork->moveInches(testPos2Fork);
        waitForMotor(motorFork);
    }
}

// Start paint rotation motor for configured number of revolutions and return start position
long startPaintRotationTwoRevolutions() {
    enablePaintRotationMotor();
    long startPos = 0;
    if (motorPaintRotation) {
        startPos = motorPaintRotation->getCurrentPosition();
        long steps = paintRotationMotorStepsPerRevOutput * TOTAL_PAINT_REVOLUTIONS;
        motorPaintRotation->moveSteps(steps);
    }
    return startPos;
}

// Start moving to waiting position (5 inches right of position 3) - non-blocking
void startMoveToWaitingPosition() {
    if (!motorX || !motorY) return;
    
    // Get current positions
    float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
    float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
    
    // Calculate target position (5 inches right of pos3)
    // Position 3 is: X = -testPos2X, Y = -testPos2Y + 0.5
    float targetX = -testPos2X + 5.0;
    float targetY = -testPos2Y + 0.5;
    
    // Calculate movement needed
    float moveX = targetX - currentX;
    float moveY = targetY - currentY;
    
    // Move both motors simultaneously (non-blocking)
    motorX->moveInches(moveX);
    motorY->moveInches(moveY);
}

// Non-blocking servo movement state
static float servoTargetAngle = -1.0;  // -1 means no target
static unsigned long lastServoUpdateTime = 0;
static const unsigned long SERVO_UPDATE_INTERVAL_MS = 10;  // Update every 10ms for smooth movement

// Start non-blocking servo movement to target angle
void startServoMoveToAngle(float angle) {
    if (servo) {
        // Constrain angle to valid range
        if (angle < 0) angle = 0;
        if (angle > 270) angle = 270;
        
        servoTargetAngle = angle;
        lastServoUpdateTime = 0;  // Reset to force immediate update
    }
}

// Update servo position incrementally (call each cycle) - non-blocking
void updateServoMovement() {
    if (!servo || servoTargetAngle < 0) return;
    
    // Check if we've reached the target
    float diff = servoTargetAngle - currentServoAngle;
    if (abs(diff) < 0.1) {
        currentServoAngle = servoTargetAngle;
        servo->write(currentServoAngle);
        servoTargetAngle = -1.0;  // Clear target
        lastServoUpdateTime = 0;  // Reset timing
        return;
    }
    
    // Check if enough time has passed for next update
    unsigned long currentTime = millis();
    if (lastServoUpdateTime == 0) {
        lastServoUpdateTime = currentTime;
        // Start moving immediately on first call
    }
    
    unsigned long deltaTime = currentTime - lastServoUpdateTime;
    if (deltaTime < SERVO_UPDATE_INTERVAL_MS) {
        return;  // Not time to update yet
    }
    
    // Calculate movement based on speed (degrees per second)
    // servoSpeed is in degrees per second
    float degreesPerMs = servoSpeed / 1000.0;  // Convert to degrees per millisecond
    float maxMovement = degreesPerMs * deltaTime;  // Maximum movement this cycle
    
    // Limit movement per update to prevent jumps (max 1 degree per update)
    const float maxStepSize = 1.0;
    if (maxMovement > maxStepSize) {
        maxMovement = maxStepSize;
    }
    
    // Ensure minimum movement if we're far from target
    if (abs(diff) > 0.5 && maxMovement < 0.2) {
        maxMovement = 0.2;  // Minimum movement to ensure progress
    }
    
    // Calculate direction and movement
    float movement = (abs(diff) < maxMovement) ? diff : (diff > 0 ? maxMovement : -maxMovement);
    
    // Update servo position
    currentServoAngle += movement;
    servo->write(currentServoAngle);
    
    // Update timing
    lastServoUpdateTime = currentTime;
}

// Check rotation count and move servo to appropriate position based on config
// Returns true if any position was triggered this call
bool updateServoPositionByRotation(long startPosition, bool resetFlags) {
    if (!motorPaintRotation) return false;
    
    static bool pos1Triggered = false;
    static bool pos2Triggered = false;
    static bool pos3Triggered = false;
    static bool pos4Triggered = false;
    
    // Reset flags if requested
    if (resetFlags) {
        pos1Triggered = false;
        pos2Triggered = false;
        pos3Triggered = false;
        pos4Triggered = false;
        return false;
    }
    
    // Calculate current rotation count
    long currentSteps = motorPaintRotation->getCurrentPosition() - startPosition;
    float currentRotations = (float)currentSteps / paintRotationMotorStepsPerRevOutput;
    
    bool positionTriggered = false;
    
    // Check position 1 (0 rotations) - trigger immediately if not already triggered
    if (!pos1Triggered && currentRotations >= SERVO_POS_1_ROTATIONS) {
        startServoMoveToAngle(SERVO_POS_1_ANGLE);
        pos1Triggered = true;
        positionTriggered = true;
    }
    
    // Check position 2
    if (!pos2Triggered && currentRotations >= SERVO_POS_2_ROTATIONS) {
        startServoMoveToAngle(SERVO_POS_2_ANGLE);
        pos2Triggered = true;
        positionTriggered = true;
    }
    
    // Check position 3
    if (!pos3Triggered && currentRotations >= SERVO_POS_3_ROTATIONS) {
        startServoMoveToAngle(SERVO_POS_3_ANGLE);
        pos3Triggered = true;
        positionTriggered = true;
    }
    
    // Check position 4
    if (!pos4Triggered && currentRotations >= SERVO_POS_4_ROTATIONS) {
        startServoMoveToAngle(SERVO_POS_4_ANGLE);
        pos4Triggered = true;
        positionTriggered = true;
    }
    
    return positionTriggered;
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ PAINTING STATE                                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void paintingState() {
    static int step = 0;
    static bool paintingStarted = false;
    static float currentAngleDeg = 0.0;
    static float previousServoSpeed = 0.0;
    
    // Update servo movement (non-blocking, call every cycle)
    updateServoMovement();
    
    // Check for cancel
    if (cycleCancelled) {
        // Stop all motors
        if (motorX) motorX->forceStop();
        if (motorY) motorY->forceStop();
        if (motorFork) motorFork->forceStop();
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
            disablePaintRotationMotor();
        }
        
        // Turn off paint gun and suction
        turnOffPaintGun();
        turnOffSuction();
        
        // Reset everything
        cycleCancelled = false;
        cyclePaused = false;
        paintingStarted = false;
        step = 0;
        servoTargetAngle = -1.0;  // Clear servo target
        lastServoUpdateTime = 0;  // Reset servo timing
        currentAngleDeg = 0.0;
        
        // Reset servo position flags
        updateServoPositionByRotation(0, true);
        
        // Move servo back to home angle
        startServoMoveToAngle(SERVO_HOME_ANGLE);
        
        // Return to test state
        setMachineState(STATE_GANTRY);
        return;
    }
    
    // Handle pause
    while (cyclePaused && !cycleCancelled) {
        updateOTA();
        delay(10);
    }
    if (cycleCancelled) return;
    
    // Initialize on first entry
    if (!paintingStarted) {
        step = 0;
        paintingStarted = true;
        currentAngleDeg = 0.0;
        previousServoSpeed = servoSpeed;
        // Reset servo position flags (not used in new sequence)
        updateServoPositionByRotation(0, true);
    }
    
    //! ************************************************************************
    //! STEP 1: TURN ON SUCTION AND PAINT GUN
    //! ************************************************************************
    if (step == 0) {
        turnOnSuction();
        if (!testModeEnabled) {
            turnOnPaintGun();
        }
        enablePaintRotationMotor();
        step = 1;
    }
    
    //! ************************************************************************
    //! STEP 2: MOVE SERVO TO START ANGLE (210°) AT CONFIGURED SPEED
    //! ************************************************************************
    else if (step == 1) {
        servoSpeed = SERVO_PAINT_MOVE_SPEED;
        startServoMoveToAngle(SERVO_PAINT_START_ANGLE);
        step = 2;
    }
    
    //! ************************************************************************
    //! STEP 3: WAIT FOR SERVO TO REACH START ANGLE
    //! ************************************************************************
    else if (step == 2) {
        if (servoTargetAngle < 0) {
            step = 3;
        }
    }
    
    //! ************************************************************************
    //! STEP 4: ROTATE TO RIGHT SIDE, THEN DWELL
    //! ************************************************************************
    else if (step == 3) {
        if (motorPaintRotation) {
            long steps = (long)((ROTATE_TO_RIGHT_DEG / 360.0) * paintRotationMotorStepsPerRevOutput);
            motorPaintRotation->moveSteps(steps);
            currentAngleDeg += ROTATE_TO_RIGHT_DEG;
        }
        step = 4;
    }
    else if (step == 4) {
        waitForMotor(motorPaintRotation);
        if (cycleCancelled) return;
        unsigned long dwellMs = (unsigned long)DWELL_RIGHT_MS;
        unsigned long startWait = millis();
        while (millis() - startWait < dwellMs) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
        step = 5;
    }
    
    //! ************************************************************************
    //! STEP 5: ROTATE TO BACK SIDE, THEN DWELL
    //! ************************************************************************
    else if (step == 5) {
        if (motorPaintRotation) {
            long steps = (long)((ROTATE_TO_BACK_DEG / 360.0) * paintRotationMotorStepsPerRevOutput);
            motorPaintRotation->moveSteps(steps);
            currentAngleDeg += ROTATE_TO_BACK_DEG;
        }
        step = 6;
    }
    else if (step == 6) {
        waitForMotor(motorPaintRotation);
        if (cycleCancelled) return;
        unsigned long dwellMs = (unsigned long)DWELL_BACK_MS;
        unsigned long startWait = millis();
        while (millis() - startWait < dwellMs) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
        step = 7;
    }
    
    //! ************************************************************************
    //! STEP 6: ROTATE TO LEFT SIDE, THEN DWELL
    //! ************************************************************************
    else if (step == 7) {
        if (motorPaintRotation) {
            long steps = (long)((ROTATE_TO_LEFT_DEG / 360.0) * paintRotationMotorStepsPerRevOutput);
            motorPaintRotation->moveSteps(steps);
            currentAngleDeg += ROTATE_TO_LEFT_DEG;
        }
        step = 8;
    }
    else if (step == 8) {
        waitForMotor(motorPaintRotation);
        if (cycleCancelled) return;
        unsigned long dwellMs = (unsigned long)DWELL_LEFT_MS;
        unsigned long startWait = millis();
        while (millis() - startWait < dwellMs) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
        step = 9;
    }
    
    //! ************************************************************************
    //! STEP 7: FINAL FULL REVOLUTION WHILE MOVING SERVO TO END ANGLE
    //! ************************************************************************
    else if (step == 9) {
        servoSpeed = SERVO_PAINT_MOVE_SPEED;
        startServoMoveToAngle(SERVO_PAINT_END_ANGLE);
        if (motorPaintRotation) {
            float baseDelta = FINAL_RETURN_TO_FRONT_DEG;
            float spinDelta = (baseDelta >= 0.0) ? FINAL_FULL_SPIN_DEG : -FINAL_FULL_SPIN_DEG;
            float finalDelta = baseDelta + spinDelta;
            long steps = (long)((finalDelta / 360.0) * paintRotationMotorStepsPerRevOutput);
            motorPaintRotation->moveSteps(steps);
            currentAngleDeg += finalDelta;
        }
        step = 10;
    }
    else if (step == 10) {
        // Wait for rotation to complete while continuing servo updates
        while (motorPaintRotation && motorPaintRotation->isMotorRunning()) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
        // Ensure servo finishes to target
        while (servoTargetAngle >= 0) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
        
        // Stop and disable paint rotation motor
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
        }
        disablePaintRotationMotor();
        
        // Turn off paint gun and suction
        turnOffPaintGun();
        turnOffSuction();
        
        // Restore servo speed
        servoSpeed = previousServoSpeed;
        
        // Reset state
        paintingStarted = false;
        step = 0;
        currentAngleDeg = 0.0;
        updateServoPositionByRotation(0, true);
        
        // Remain at end angle; return to test state
        setMachineState(STATE_GANTRY);
    }
}
