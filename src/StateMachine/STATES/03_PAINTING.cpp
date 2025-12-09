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
    long targetSteps = paintRotationMotorStepsPerRevOutput * revolutions;
    
    while (isStepperRunning()) {
        long currentSteps = getStepperPosition() - startPosition;
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
    long currentSteps = getStepperPosition() - startPosition;
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
    static unsigned long waitingPositionReachedTime = 0;
    static float savedServoSpeed = 0.0;
    static unsigned long waitStartTime = 0;
    static bool servoAt210Complete = false;
    static bool servoAt180Complete = false;
    static bool rotationToBackStarted = false;
    static bool rotationToRightStarted = false;
    static bool finalRotationStarted = false;
    
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
        rotationToBackStarted = false;
        rotationToRightStarted = false;
        finalRotationStarted = false;
        
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
        rotationToBackStarted = false;
        rotationToRightStarted = false;
        finalRotationStarted = false;
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
    //! STEP 3: TURN ON PAINT GUN
    //! ************************************************************************
    else if (step == 2) {
        if (!testModeEnabled) {
            turnOnPaintGun();
        }
        step = 3;
    }
    
    //! ************************************************************************
    //! STEP 4: SERVO TO 210 DEGREES AT 40 SPEED, THEN RESTORE DASHBOARD SPEED
    //! ************************************************************************
    else if (step == 3) {
        // Save current servo speed and set to fast speed
        if (savedServoSpeed == 0.0) {
            savedServoSpeed = servoSpeed;
            servoSpeed = SERVO_FAST_SPEED;
        }
        
        // Start moving servo to 210 degrees (non-blocking)
        if (!servoAt210Complete) {
            startServoMoveToAngle(210.0);
            servoAt210Complete = true;
        }
        
        // Wait for servo to reach target
        if (servoTargetAngle < 0) {
            // Servo movement complete, restore dashboard speed
            servoSpeed = savedServoSpeed;
            savedServoSpeed = 0.0;
            step = 4;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 5: ROTATE TO LEFT SIDE (90 DEGREE CW TURN) - 2400 STEPS
    //! ************************************************************************
    else if (step == 4) {
        // Initialize paint rotation motor if not already started
        if (paintMotorStartPosition == 0) {
            enablePaintRotationMotor();
            paintMotorStartPosition = getStepperPosition();
            moveStepper(2400);  // 90 degrees = 0.25 rev = 2400 steps
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 5;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 6: WAIT ON LEFT SIDE FOR 500MS
    //! ************************************************************************
    else if (step == 5) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= LEFT_SIDE_WAIT_MS) {
            step = 6;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 7: ROTATE TO BACK (180 CW FROM START) - 4800 STEPS TOTAL
    //! ************************************************************************
    else if (step == 6) {
        // Rotate to 180 degrees (2400 more steps from 90, 4800 total from start)
        if (!rotationToBackStarted) {
            moveStepper(2400);
            rotationToBackStarted = true;
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 7;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 8: WAIT ON BACK FOR 1000MS
    //! ************************************************************************
    else if (step == 7) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= BACK_SIDE_WAIT_MS) {
            step = 8;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 9: ROTATE TO RIGHT (270 CW FROM START) - 7200 STEPS TOTAL
    //! ************************************************************************
    else if (step == 8) {
        // Rotate to 270 degrees (2400 more steps from 180, 7200 total from start)
        if (!rotationToRightStarted) {
            moveStepper(2400);
            rotationToRightStarted = true;
        }
        
        // Wait for rotation to complete
        if (!isStepperRunning()) {
            step = 9;
            waitStartTime = millis();
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 10: WAIT ON RIGHT FOR 500MS
    //! ************************************************************************
    else if (step == 9) {
        unsigned long elapsedTime = millis() - waitStartTime;
        if (elapsedTime >= RIGHT_SIDE_WAIT_MS) {
            step = 10;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 11: MOVE SERVO TO 180 DEGREES AND ROTATE 450 DEGREES CW (1.25 REV = 12000 STEPS)
    //! ************************************************************************
    else if (step == 10) {
        // Start moving servo to 180 degrees (non-blocking)
        if (!servoAt180Complete) {
            startServoMoveToAngle(180.0);
            servoAt180Complete = true;
        }
        
        // Start rotation (can happen simultaneously with servo movement)
        if (!finalRotationStarted && !isStepperRunning()) {
            moveStepper(12000);  // 450 degrees = 1.25 rev = 12000 steps
            finalRotationStarted = true;
        }
        
        // Wait for both servo and rotation to complete
        bool rotationComplete = !isStepperRunning();
        bool servoComplete = servoTargetAngle < 0;
        
        if (rotationComplete && servoComplete) {
            step = 11;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 12: TURN OFF PAINT GUN AND SUCTION, CLEANUP AND RETURN TO GANTRY STATE
    //! ************************************************************************
    else if (step == 11) {
        // Turn off paint gun and suction
        turnOffPaintGun();
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
        rotationToBackStarted = false;
        rotationToRightStarted = false;
        finalRotationStarted = false;
        
        // Move servo back to home angle
        startServoMoveToAngle(SERVO_HOME_ANGLE);
        
        // Return to gantry state
        setMachineState(STATE_GANTRY);
    }
}
