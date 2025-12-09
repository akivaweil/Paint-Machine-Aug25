#include <Arduino.h>
#include "StateMachine/STATES/03_PAINTING.h"
#include "../../config/Config.h"
#include "../../config/Painting_Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"
#include "ServoControl.h"
#include "../../config/Pin_Definitions.h"
#include "Paint_Motor_Controller.h"

// External motor instances (defined in Web_Manager.cpp)
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;

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

// Paint rotation motor speed and acceleration
extern long motorSpeedPaintRotation;
extern long motorAccelPaintRotation;

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
    moveYWithSpeedAdjustment(moveY);
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
    
    // Limit movement per update to prevent jumps (max 1.25 degrees per update for faster movement)
    const float maxStepSize = 1.25;
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

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ PAINTING STATE                                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void paintingState() {
    static int step = 0;
    static bool paintingStarted = false;
    static long paintMotorStartPosition = 0;
    static unsigned long waitingPositionReachedTime = 0;
    static float savedServoSpeed = 0.0;
    static unsigned long waitStartTime = 0;
    static bool servoAt210Complete = false;
    static bool servoAt180Complete = false;
    static bool rotationToLeftStarted = false;
    static bool rotationToBackLeftStarted = false;
    static bool rotationToBackStarted = false;
    static bool rotationToBackRightStarted = false;
    static bool rotationToRightStarted = false;
    static bool finalRotationStarted = false;
    static bool servoAtHomeComplete = false;
    static unsigned long servoStartTime = 0;
    static unsigned long servoReachedPaintingAngleTime = 0;
    static bool initial180RotationStarted = false;
    
    // Update servo movement (non-blocking, call every cycle)
    updateServoMovement();
    
    // Check for cancel
    if (cycleCancelled) {
        // Stop all motors
        if (motorX) motorX->forceStop();
        if (motorY) motorY->forceStop();
        if (motorFork) motorFork->forceStop();
        stopStepper();
        disablePaintRotationMotor();
        
        // Turn off paint gun and suction
        turnOffPaintGun();
        turnOffSuction();
        
        // Restore servo speed if it was changed
        if (savedServoSpeed > 0) {
            servoSpeed = savedServoSpeed;
            savedServoSpeed = 0.0;
        }
        
        // Reset everything
        cycleCancelled = false;
        cyclePaused = false;
        paintingStarted = false;
        step = 0;
        servoTargetAngle = -1.0;  // Clear servo target
        lastServoUpdateTime = 0;  // Reset servo timing
        waitingPositionReachedTime = 0;
        paintMotorStartPosition = 0;
        savedServoSpeed = 0.0;
        waitStartTime = 0;
        servoAt210Complete = false;
        servoAt180Complete = false;
        servoAtHomeComplete = false;
        rotationToLeftStarted = false;
        rotationToBackLeftStarted = false;
        rotationToBackStarted = false;
        rotationToBackRightStarted = false;
        rotationToRightStarted = false;
        finalRotationStarted = false;
        servoStartTime = 0;
        servoReachedPaintingAngleTime = 0;
        initial180RotationStarted = false;
        
        // Move servo back to home angle
        startServoMoveToAngle(SERVO_HOME_ANGLE);
        
        // Return to gantry state
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
        waitingPositionReachedTime = 0;
        paintMotorStartPosition = 0;
        savedServoSpeed = 0.0;
        waitStartTime = 0;
        servoAt210Complete = false;
        servoAt180Complete = false;
        rotationToLeftStarted = false;
        rotationToBackLeftStarted = false;
        rotationToBackStarted = false;
        rotationToBackRightStarted = false;
        rotationToRightStarted = false;
        finalRotationStarted = false;
        servoAtHomeComplete = false;
        servoStartTime = 0;
        servoReachedPaintingAngleTime = 0;
        initial180RotationStarted = false;
    }
    
    //! ************************************************************************
    //! STEP 1: TURN ON SUCTION AND MOVE GANTRY TO WAITING POSITION (NON-BLOCKING)
    //! ************************************************************************
    if (step == 0) {
        // Turn on suction
        turnOnSuction();
        
        // Start moving to waiting position (non-blocking)
        startMoveToWaitingPosition();
        
        step = 1;
    }
    
    //! ************************************************************************
    //! STEP 2: WAIT FOR GANTRY TO REACH WAITING POSITION, THEN WAIT 250MS
    //! ************************************************************************
    else if (step == 1) {
        // Wait for motors to reach waiting position
        bool motorsRunning = (motorX && motorX->isMotorRunning()) || (motorY && motorY->isMotorRunning());
        while (motorsRunning) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            
            motorsRunning = (motorX && motorX->isMotorRunning()) || (motorY && motorY->isMotorRunning());
            delay(1);
        }
        
        // Record time when waiting position is reached
        if (waitingPositionReachedTime == 0) {
            waitingPositionReachedTime = millis();
        }
        
        // Wait 250ms after motors reach waiting position
        unsigned long elapsedTime = millis() - waitingPositionReachedTime;
        if (elapsedTime >= WAITING_POSITION_DELAY_MS) {
            step = 2;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 3: MOVE SERVO TO PAINTING ANGLE, THEN TURN ON PAINT GUN
    //! ************************************************************************
    else if (step == 2) {
        // Save current servo speed and set to fast speed
        if (savedServoSpeed == 0.0) {
            savedServoSpeed = servoSpeed;
            servoSpeed = SERVO_FAST_SPEED;
        }
        
        // Initialize paint rotation motor if not already started
        if (paintMotorStartPosition == 0) {
            // Set speed and acceleration from dashboard settings
            setStepperSpeed(motorSpeedPaintRotation);
            setStepperAcceleration(motorAccelPaintRotation);
            enablePaintRotationMotor();
            resetStepperPosition();  // Reset to 0 to ensure clean start
            paintMotorStartPosition = getStepperPosition();  // Should be 0 now
        }
        
        // Start moving servo to painting angle (non-blocking)
        if (!servoAt210Complete) {
            startServoMoveToAngle(SERVO_PAINTING_ANGLE);
            servoAt210Complete = true;
            servoStartTime = millis();  // Record when servo started moving
        }
        
        // Wait for servo to reach painting angle before turning on paint gun
        if (servoTargetAngle < 0) {
            // Servo has reached painting angle, turn on paint gun
            if (!testModeEnabled) {
                turnOnPaintGun();
            }
            
            // Record time when servo reached painting angle
            if (servoReachedPaintingAngleTime == 0) {
                servoReachedPaintingAngleTime = millis();
            }
            
            // Restore dashboard speed
            servoSpeed = savedServoSpeed;
            savedServoSpeed = 0.0;
            
            step = 3;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 4: START INITIAL 180-DEGREE ROTATION AND CONTINUE
    //! ************************************************************************
    else if (step == 3) {
        // 200ms after servo reached painting angle, start 180-degree rotation
        if (!initial180RotationStarted && servoReachedPaintingAngleTime > 0) {
            unsigned long elapsedTime = millis() - servoReachedPaintingAngleTime;
            if (elapsedTime >= INITIAL_ROTATION_DELAY_MS) {
                moveStepper(paintRotationMotorStepsPerRevOutput / 2);  // 180 degrees = 0.5 rev
                initial180RotationStarted = true;
            }
        }
        
        // Wait for initial rotation delay to pass (servo already at angle, paint gun already on)
        if (initial180RotationStarted || (servoReachedPaintingAngleTime > 0 && (millis() - servoReachedPaintingAngleTime) >= INITIAL_ROTATION_DELAY_MS)) {
            step = 4;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 5: WAIT FOR INITIAL 180-DEGREE ROTATION TO COMPLETE, THEN CONTINUE TO LEFT
    //! ************************************************************************
    else if (step == 4) {
        // Wait for initial 180-degree rotation to complete (if it was started)
        if (initial180RotationStarted && isStepperRunning()) {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        } else {
            // Initial rotation complete (or wasn't needed), continue to left side
            step = 5;
        }
    }
    
    //! ************************************************************************
    //! STEP 7: ROTATE TO LEFT SIDE (90 DEGREE CCW TURN)
    //! ************************************************************************
    else if (step == 5) {
        // Rotate to 90 degrees CCW (90 degrees counter-clockwise from start)
        if (!rotationToLeftStarted) {
            moveStepper(-paintRotationMotorStepsPerRevOutput / 4);  // 90 degrees CCW = -0.25 rev
            rotationToLeftStarted = true;
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 6;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 8: WAIT ON LEFT SIDE FOR 500MS
    //! ************************************************************************
    else if (step == 6) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= LEFT_SIDE_WAIT_MS) {
            step = 7;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 9: ROTATE TO BACK LEFT (135 DEGREE CW FROM START)
    //! ************************************************************************
    else if (step == 7) {
        // Rotate to 135 degrees (45 more degrees from 90, 135 total from start)
        if (!rotationToBackLeftStarted) {
            moveStepper(paintRotationMotorStepsPerRevOutput / 8);  // 45 degrees = 0.125 rev
            rotationToBackLeftStarted = true;
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 8;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 10: WAIT ON BACK LEFT FOR 500MS
    //! ************************************************************************
    else if (step == 8) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= LEFT_SIDE_WAIT_MS) {
            step = 9;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 11: ROTATE TO BACK (180 CW FROM START)
    //! ************************************************************************
    else if (step == 9) {
        // Rotate to 180 degrees (45 more degrees from 135, 180 total from start)
        if (!rotationToBackStarted) {
            moveStepper(paintRotationMotorStepsPerRevOutput / 8);  // 45 degrees = 0.125 rev
            rotationToBackStarted = true;
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 10;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 12: WAIT ON BACK FOR 500MS
    //! ************************************************************************
    else if (step == 10) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= LEFT_SIDE_WAIT_MS) {
            step = 11;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 13: ROTATE TO BACK RIGHT (225 DEGREE CW FROM START)
    //! ************************************************************************
    else if (step == 11) {
        // Rotate to 225 degrees (45 more degrees from 180, 225 total from start)
        if (!rotationToBackRightStarted) {
            moveStepper(paintRotationMotorStepsPerRevOutput / 8);  // 45 degrees = 0.125 rev
            rotationToBackRightStarted = true;
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 12;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 14: WAIT ON BACK RIGHT FOR 500MS
    //! ************************************************************************
    else if (step == 12) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= LEFT_SIDE_WAIT_MS) {
            step = 13;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 15: ROTATE TO RIGHT (270 CW FROM START)
    //! ************************************************************************
    else if (step == 13) {
        // Rotate to 270 degrees (45 more degrees from 225, 270 total from start)
        if (!rotationToRightStarted) {
            moveStepper(paintRotationMotorStepsPerRevOutput / 8);  // 45 degrees = 0.125 rev
            rotationToRightStarted = true;
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 14;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 16: WAIT ON RIGHT FOR 500MS
    //! ************************************************************************
    else if (step == 14) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= RIGHT_SIDE_WAIT_MS) {
            step = 15;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 17: MOVE SERVO TO PAINTING ANGLE AND ROTATE 810 DEGREES CW (2.25 REV)
    //! ************************************************************************
    else if (step == 15) {
        // Start moving servo to painting angle (non-blocking)
        if (!servoAt180Complete) {
            startServoMoveToAngle(SERVO_PAINTING_ANGLE);
            servoAt180Complete = true;
        }
        
        // Start rotation (can happen simultaneously with servo movement)
        if (!finalRotationStarted && !isStepperRunning()) {
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * 2.25));  // 810 degrees = 2.25 rev
            finalRotationStarted = true;
        }
        
        // Wait for both servo and rotation to complete
        bool rotationComplete = !isStepperRunning();
        bool servoComplete = servoTargetAngle < 0;
        
        if (rotationComplete && servoComplete) {
            step = 16;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 18: TURN OFF PAINT GUN AND MOVE SERVO TO SERVO HOME POSITION
    //! ************************************************************************
    else if (step == 16) {
        // Turn off paint gun before moving servo to home
        if (!servoAtHomeComplete) {
            turnOffPaintGun();
        }
        
        // Start moving servo to home angle (non-blocking)
        if (!servoAtHomeComplete) {
            startServoMoveToAngle(SERVO_HOME_ANGLE);
            servoAtHomeComplete = true;
        }
        
        // Wait for servo to reach target
        if (servoTargetAngle < 0) {
            // Servo movement complete
            step = 17;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 19: TURN OFF SUCTION, CLEANUP AND RETURN TO GANTRY STATE
    //! ************************************************************************
    else if (step == 17) {
        // Turn off suction
        turnOffSuction();
        
        // Stop and disable paint rotation motor
        stopStepper();
        disablePaintRotationMotor();
        
        // Reset state
        paintingStarted = false;
        step = 0;
        waitingPositionReachedTime = 0;
        paintMotorStartPosition = 0;
        savedServoSpeed = 0.0;
        waitStartTime = 0;
        servoAt210Complete = false;
        servoAt180Complete = false;
        servoAtHomeComplete = false;
        rotationToLeftStarted = false;
        rotationToBackLeftStarted = false;
        rotationToBackStarted = false;
        rotationToBackRightStarted = false;
        rotationToRightStarted = false;
        finalRotationStarted = false;
        servoStartTime = 0;
        servoReachedPaintingAngleTime = 0;
        initial180RotationStarted = false;
        
        // Return to gantry state
        setMachineState(STATE_GANTRY);
    }
}
