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
    unsigned long startTime = millis();
    const unsigned long MAX_WAIT_MS = 30000;  // 30 second timeout to prevent infinite loops
    long lastPosition = startPosition;
    unsigned long lastPositionChangeTime = millis();
    
    // Wait until we've reached the target position
    // Check position very frequently to catch target at high speeds
    while (true) {
        // Timeout check to prevent infinite loops
        if (millis() - startTime > MAX_WAIT_MS) {
            // Force stop if timeout
            motorPaintRotation->forceStop();
            break;
        }
        
        // Check position multiple times per loop for high-speed accuracy
        long currentPosition = motorPaintRotation->getCurrentPosition();
        long currentSteps = abs(currentPosition - startPosition);  // Use abs to handle any direction
        
        // Check if position is changing - if not changing for 2 seconds and motor says it's running, force stop
        if (currentPosition != lastPosition) {
            lastPosition = currentPosition;
            lastPositionChangeTime = millis();
        } else if (motorPaintRotation->isMotorRunning() && (millis() - lastPositionChangeTime > 2000)) {
            // Position hasn't changed in 2 seconds but motor says it's running - force stop
            motorPaintRotation->forceStop();
            break;
        }
        
        // If we've reached or exceeded the target, force stop immediately and break
        // Check if we're at or past target
        if (currentSteps >= targetSteps) {
            // Force stop immediately - don't wait for servo
            motorPaintRotation->forceStop();
            // Wait a moment for stop to take effect, then verify
            delay(10);
            // Force stop again to ensure it's stopped
            if (motorPaintRotation->isMotorRunning()) {
                motorPaintRotation->forceStop();
            }
            break;
        }
        
        // Also check if we're very close to target (within 200 steps) and motor is running
        // This helps prevent overshoot at high speeds by checking more frequently
        if (currentSteps >= (targetSteps - 200) && motorPaintRotation->isMotorRunning()) {
            // We're close - check position again immediately
            long quickCheckPos = motorPaintRotation->getCurrentPosition();
            long quickCheckSteps = abs(quickCheckPos - startPosition);
            if (quickCheckSteps >= targetSteps) {
                motorPaintRotation->forceStop();
                delay(10);
                if (motorPaintRotation->isMotorRunning()) {
                    motorPaintRotation->forceStop();
                }
                break;
            }
        }
        
        // If motor stopped but we haven't reached target, something went wrong - break to avoid infinite loop
        if (!motorPaintRotation->isMotorRunning() && currentSteps < targetSteps) {
            break;
        }
        
        updateServoMovement();  // Update servo while waiting
        updateServoPositionByRotation(startPosition, false);  // Update servo position based on rotation
        updateOTA();
        if (cycleCancelled) return;
        // No delay - check as fast as possible for high-speed accuracy
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
    static long paintMotorStartPosition = 0;
    static unsigned long servoStartTime = 0;
    static unsigned long waitingPositionReachedTime = 0;
    static bool paintGunTurnedOn = false;
    static bool paintGunTurnedOff = false;
    
    // Update servo movement (non-blocking, call every cycle)
    updateServoMovement();
    
    // Update servo position based on rotation count (if motor has started)
    if (paintingStarted && paintMotorStartPosition != 0) {
        updateServoPositionByRotation(paintMotorStartPosition, false);
    }
    
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
        servoStartTime = 0;
        waitingPositionReachedTime = 0;
        paintGunTurnedOn = false;
        paintGunTurnedOff = false;
        paintMotorStartPosition = 0;
        
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
        // Reset servo position flags
        updateServoPositionByRotation(0, true);
        // Initialize servo to position 1 (0 rotations)
        startServoMoveToAngle(SERVO_POS_1_ANGLE);
    }
    
    //! ************************************************************************
    //! STEP 0: DO FULL 360-DEGREE ROTATION BEFORE ANYTHING ELSE
    //! ************************************************************************
    if (step == 0) {
        // Enable paint rotation motor
        enablePaintRotationMotor();
        
        // Get start position
        long initialStartPos = 0;
        if (motorPaintRotation) {
            initialStartPos = motorPaintRotation->getCurrentPosition();
            // Move 1 full revolution (360 degrees) - use exact steps per rev
            long steps = paintRotationMotorStepsPerRevOutput;
            // Don't use moveSteps - instead we'll check position and stop manually
            // This gives us better control at high speeds
            motorPaintRotation->moveSteps(steps);
        }
        
        // Wait for rotation to complete (motor will stop immediately when target reached)
        // Pass 1.0 for exactly one revolution
        waitForPaintRotationRevolutions(initialStartPos, 1.0);
        if (cycleCancelled) return;
        
        // Ensure motor is stopped
        if (motorPaintRotation && motorPaintRotation->isMotorRunning()) {
            motorPaintRotation->forceStop();
        }
        
        // Wait for servo to finish moving (separate from motor stop)
        while (servoTargetAngle >= 0) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
        
        step = 1;
    }
    
    //! ************************************************************************
    //! STEP 1: TURN ON SUCTION, START PAINT ROTATION, MOVE TO WAITING POSITION (ALL NON-BLOCKING)
    //!         SERVO POSITIONS WILL BE AUTOMATICALLY UPDATED BASED ON ROTATION COUNT
    //!         TURN ON PAINT GUN 0.5 SECONDS AFTER WAITING POSITION IS REACHED
    //! ************************************************************************
    else if (step == 1) {
        // Turn on suction (non-blocking)
        turnOnSuction();
        
        // Start paint rotation motor for configured number of revolutions (non-blocking)
        paintMotorStartPosition = startPaintRotationTwoRevolutions();
        
        // Start moving to waiting position (non-blocking)
        startMoveToWaitingPosition();
        
        step = 2;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR GANTRY TO REACH WAITING POSITION, THEN WAIT 250MS BEFORE TURNING ON PAINT GUN
    //! ************************************************************************
    else if (step == 2) {
        // Wait for motors to reach waiting position
        bool motorsRunning = (motorX && motorX->isMotorRunning()) || (motorY && motorY->isMotorRunning());
        while (motorsRunning) {
            updateServoMovement();  // Update servo while waiting
            updateServoPositionByRotation(paintMotorStartPosition, false);  // Update servo position based on rotation
            updateOTA();
            if (cycleCancelled) return;
            
            // Update motors running status
            motorsRunning = (motorX && motorX->isMotorRunning()) || (motorY && motorY->isMotorRunning());
            
            delay(1);
        }
        
        // Record time when waiting position is reached
        if (waitingPositionReachedTime == 0) {
            waitingPositionReachedTime = millis();
        }
        
        // Wait 250ms after motors reach waiting position before turning on paint gun
        while (!paintGunTurnedOn && waitingPositionReachedTime > 0) {
            unsigned long elapsedTime = millis() - waitingPositionReachedTime;
            if (elapsedTime >= 250) {
                if (!testModeEnabled) {
                    turnOnPaintGun();
                }
                paintGunTurnedOn = true;
            } else {
                updateServoMovement();  // Update servo while waiting
                updateServoPositionByRotation(paintMotorStartPosition, false);  // Update servo position based on rotation
                updateOTA();
                if (cycleCancelled) return;
                delay(1);
            }
        }
        
        if (cycleCancelled) return;
        step = 3;
    }
    
    //! ************************************************************************
    //! STEP 3: WAIT FOR PAINT_GUN_OFF_REVOLUTIONS TO TURN OFF PAINT GUN
    //!         SERVO POSITIONS ARE AUTOMATICALLY UPDATED BASED ON ROTATION COUNT
    //! ************************************************************************
    else if (step == 3) {
        // Wait for configured number of revolutions remaining to turn off paint gun
        if (!paintGunTurnedOff) {
            float paintGunOffPosition = TOTAL_PAINT_REVOLUTIONS - PAINT_GUN_OFF_REVOLUTIONS;
            waitForPaintRotationRevolutions(paintMotorStartPosition, paintGunOffPosition);
            if (cycleCancelled) return;
            
            // Turn off paint gun after configured revolutions remaining
            if (!testModeEnabled && paintGunTurnedOn) {
                turnOffPaintGun();
            }
            paintGunTurnedOff = true;
        }
        
        step = 4;
    }
    
    //! ************************************************************************
    //! STEP 4: WAIT FOR PAINT MOTOR TO COMPLETE TOTAL REVOLUTIONS
    //! ************************************************************************
    else if (step == 4) {
        waitForPaintRotationRevolutions(paintMotorStartPosition, TOTAL_PAINT_REVOLUTIONS);
        if (cycleCancelled) return;
        step = 5;
    }
    
    //! ************************************************************************
    //! STEP 5: WAIT FOR PAINT MOTOR TO FINISH, THEN TURN OFF EVERYTHING
    //! ************************************************************************
    else if (step == 5) {
        waitForMotor(motorPaintRotation);
        if (cycleCancelled) return;
        
        // Stop and disable paint rotation motor
        if (motorPaintRotation) {
            motorPaintRotation->forceStop();
        }
        disablePaintRotationMotor();
        
        // Turn off paint gun and suction
        turnOffPaintGun();
        turnOffSuction();
        
        // Reset state
        paintingStarted = false;
        step = 0;
        servoStartTime = 0;
        waitingPositionReachedTime = 0;
        paintGunTurnedOn = false;
        paintGunTurnedOff = false;
        paintMotorStartPosition = 0;
        
        // Reset servo position flags
        updateServoPositionByRotation(0, true);
        
        // Move servo back to home angle
        startServoMoveToAngle(SERVO_HOME_ANGLE);
        
        // Return to test state
        setMachineState(STATE_GANTRY);
    }
}
