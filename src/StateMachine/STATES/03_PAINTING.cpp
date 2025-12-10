#include <Arduino.h>
#include "StateMachine/STATES/03_PAINTING.h"
#include "../../config/Config.h"
#include "../../config/Painting_Config.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "Web_Manager.h"
#include "ServoControl.h"
#include "../../config/Pin_Definitions.h"
#include "Paint_Motor_Controller.h"

//* ************************************************************************
//* ************************ PAINTING STATE ********************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ HELPER FUNCTIONS                                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Helper function to move Y-axis with speed adjustment based on direction
static void moveYWithSpeedAdjustment(float inches) {
    if (!motorY) return;
    
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
    
    // Calculate target position (offset from pos3)
    // Position 3 is: X = -testPos2X, Y = -testPos2Y + 0.5
    float targetX = -testPos2X + STEP1_WAITING_POSITION_X_OFFSET_INCHES;
    float targetY = -testPos2Y + STEP1_WAITING_POSITION_Y_OFFSET_INCHES;
    
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
    if (abs(diff) < SERVO_TARGET_REACHED_THRESHOLD_DEG) {
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
    
    // Limit movement per update to prevent jumps
    if (maxMovement > SERVO_MAX_STEP_SIZE_DEG) {
        maxMovement = SERVO_MAX_STEP_SIZE_DEG;
    }
    
    // Ensure minimum movement if we're far from target
    if (abs(diff) > SERVO_FAR_FROM_TARGET_THRESHOLD_DEG && maxMovement < SERVO_MIN_MOVEMENT_DEG) {
        maxMovement = SERVO_MIN_MOVEMENT_DEG;  // Minimum movement to ensure progress
    }
    
    // Calculate direction and movement
    float movement = (abs(diff) < maxMovement) ? diff : (diff > 0 ? maxMovement : -maxMovement);
    
    // Update servo position
    currentServoAngle += movement;
    
    // Prevent overshooting the target
    if ((movement > 0 && currentServoAngle > servoTargetAngle) || 
        (movement < 0 && currentServoAngle < servoTargetAngle)) {
        currentServoAngle = servoTargetAngle;
    }
    
    // Constrain angle to valid range (0-270)
    if (currentServoAngle < 0) currentServoAngle = 0;
    if (currentServoAngle > 270) currentServoAngle = 270;
    
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
    static bool backLeftWaitComplete = false;
    static bool backLeftServoTo160Complete = false;
    static bool backLeftServoBackComplete = false;
    static bool backRightWaitComplete = false;
    static bool backRightServoTo160Complete = false;
    static bool backRightServoBackComplete = false;
    static bool servoToHomeStarted = false;
    static bool servoReachedHomeInStep17 = false;
    static long step15RotationStartPosition = 0;
    static bool step15ServoMovedTo170 = false;
    static bool step17ServoTo190Started = false;
    static bool step17ServoTo190Complete = false;
    static unsigned long step17InitialAngleReachedTime = 0;
    static bool step17ServoToHomeStarted = false;
    static bool step17ServoToHomeComplete = false;
    static bool step17ServoBackComplete = false;
    static bool step15PaintGunTurnedOff = false;
    static unsigned long step15PaintGunOffTime = 0;
    static bool step18PaintGunTurnedOff = false;
    static unsigned long step18PaintGunOffTime = 0;
    
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
        backLeftWaitComplete = false;
        backLeftServoTo160Complete = false;
        backLeftServoBackComplete = false;
        backRightWaitComplete = false;
        backRightServoTo160Complete = false;
        backRightServoBackComplete = false;
        servoToHomeStarted = false;
        servoReachedHomeInStep17 = false;
        step15RotationStartPosition = 0;
        step15ServoMovedTo170 = false;
        step17ServoTo190Started = false;
        step17ServoTo190Complete = false;
        step17InitialAngleReachedTime = 0;
        step17ServoToHomeStarted = false;
        step17ServoToHomeComplete = false;
        step17ServoBackComplete = false;
        step15PaintGunTurnedOff = false;
        step15PaintGunOffTime = 0;
        step18PaintGunTurnedOff = false;
        step18PaintGunOffTime = 0;
        
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
        backLeftWaitComplete = false;
        backLeftServoTo160Complete = false;
        backLeftServoBackComplete = false;
        backRightWaitComplete = false;
        backRightServoTo160Complete = false;
        backRightServoBackComplete = false;
        servoToHomeStarted = false;
        servoReachedHomeInStep17 = false;
        step15RotationStartPosition = 0;
        step15ServoMovedTo170 = false;
        step17ServoTo190Started = false;
        step17ServoTo190Complete = false;
        step17InitialAngleReachedTime = 0;
        step17ServoToHomeStarted = false;
        step17ServoToHomeComplete = false;
        step17ServoBackComplete = false;
        step15PaintGunTurnedOff = false;
        step15PaintGunOffTime = 0;
        step18PaintGunTurnedOff = false;
        step18PaintGunOffTime = 0;
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
        
        // Wait after motors reach waiting position
        unsigned long elapsedTime = millis() - waitingPositionReachedTime;
        if (elapsedTime >= STEP2_WAITING_POSITION_DELAY_MS) {
            step = 2;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 3: MOVE SERVO TO PAINTING ANGLE AND START INITIAL 180-DEGREE ROTATION (SIMULTANEOUS)
    //! ************************************************************************
    else if (step == 2) {
        // Save current servo speed and set to fast speed
        if (savedServoSpeed == 0.0) {
            savedServoSpeed = servoSpeed;
            servoSpeed = STEP3_SERVO_FAST_SPEED;
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
        
        // Start initial 180-degree rotation simultaneously (non-blocking)
        if (!initial180RotationStarted) {
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * STEP4_INITIAL_ROTATION_REV));
            initial180RotationStarted = true;
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
    //! STEP 4: WAIT FOR INITIAL 180-DEGREE ROTATION TO COMPLETE
    //! ************************************************************************
    else if (step == 3) {
        // Wait for initial rotation to complete
        if (!isStepperRunning()) {
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
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * STEP5_LEFT_ROTATION_REV));
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
        if (elapsedTime >= STEP6_LEFT_SIDE_WAIT_MS) {
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
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * STEP7_BACK_LEFT_ROTATION_REV));
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
    //! STEP 10: WAIT ON BACK LEFT FOR 500MS, THEN MOVE SERVO TO 160° AND BACK
    //! ************************************************************************
    else if (step == 8) {
        // Wait for initial wait time
        if (!backLeftWaitComplete) {
            unsigned long elapsedTime = millis() - waitStartTime;
            if (elapsedTime >= STEP8_BACK_LEFT_WAIT_MS) {
                backLeftWaitComplete = true;
                // Start moving servo to back angle
                startServoMoveToAngle(STEP8_SERVO_BACK_ANGLE_DEG);
            }
        }
        // Wait for servo to reach 160 degrees
        else if (!backLeftServoTo160Complete) {
            if (servoTargetAngle < 0) {
                backLeftServoTo160Complete = true;
                // Start moving servo back to painting angle
                startServoMoveToAngle(SERVO_PAINTING_ANGLE);
            }
        }
        // Wait for servo to return to painting angle
        else if (!backLeftServoBackComplete) {
            if (servoTargetAngle < 0) {
                backLeftServoBackComplete = true;
                step = 9;
            }
        }
        
        updateServoMovement();
        updateOTA();
        if (cycleCancelled) return;
        delay(1);
    }
    
    //! ************************************************************************
    //! STEP 11: ROTATE TO BACK (180 CW FROM START)
    //! ************************************************************************
    else if (step == 9) {
        // Rotate to 180 degrees (45 more degrees from 135, 180 total from start)
        if (!rotationToBackStarted) {
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * STEP9_BACK_ROTATION_REV));
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
        if (elapsedTime >= STEP10_BACK_SIDE_WAIT_MS) {
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
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * STEP11_BACK_RIGHT_ROTATION_REV));
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
    //! STEP 14: WAIT ON BACK RIGHT FOR 500MS, THEN MOVE SERVO TO 160° AND BACK
    //! ************************************************************************
    else if (step == 12) {
        // Wait for initial wait time
        if (!backRightWaitComplete) {
            unsigned long elapsedTime = millis() - waitStartTime;
            if (elapsedTime >= STEP12_BACK_RIGHT_WAIT_MS) {
                backRightWaitComplete = true;
                // Start moving servo to back angle
                startServoMoveToAngle(STEP12_SERVO_BACK_ANGLE_DEG);
            }
        }
        // Wait for servo to reach 160 degrees
        else if (!backRightServoTo160Complete) {
            if (servoTargetAngle < 0) {
                backRightServoTo160Complete = true;
                // Start moving servo back to painting angle
                startServoMoveToAngle(SERVO_PAINTING_ANGLE);
            }
        }
        // Wait for servo to return to painting angle
        else if (!backRightServoBackComplete) {
            if (servoTargetAngle < 0) {
                backRightServoBackComplete = true;
                step = 13;
            }
        }
        
        updateServoMovement();
        updateOTA();
        if (cycleCancelled) return;
        delay(1);
    }
    
    //! ************************************************************************
    //! STEP 15: ROTATE TO RIGHT (270 CW FROM START)
    //! ************************************************************************
    else if (step == 13) {
        // Rotate to 270 degrees (45 more degrees from 225, 270 total from start)
        if (!rotationToRightStarted) {
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * STEP13_RIGHT_ROTATION_REV));
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
        if (elapsedTime >= STEP14_RIGHT_SIDE_WAIT_MS) {
            step = 15;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 17: TURN OFF PAINT GUN FOR 500MS TO REFILL, THEN TURN BACK ON
    //! ************************************************************************
    else if (step == 15) {
        // Turn off paint gun
        if (!step15PaintGunTurnedOff) {
            turnOffPaintGun();
            step15PaintGunTurnedOff = true;
            step15PaintGunOffTime = millis();
        }
        
        // Wait for delay period
        unsigned long elapsedTime = millis() - step15PaintGunOffTime;
        if (elapsedTime >= STEP15_PAINT_GUN_OFF_DELAY_MS) {
            // Turn paint gun back on
            if (!testModeEnabled) {
                turnOnPaintGun();
            }
            step = 16;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 18: MOVE SERVO TO 210°, ROTATE 2.25 REVS, MOVE SERVO TO 170° AFTER 1 REV
    //! ************************************************************************
    else if (step == 16) {
        // Start moving servo to first angle (210°) (non-blocking)
        if (!servoAt180Complete) {
            startServoMoveToAngle(STEP15_FIRST_REV_SERVO_ANGLE_DEG);
            servoAt180Complete = true;
        }
        
        // Start rotation once servo reaches angle (can happen simultaneously)
        if (!finalRotationStarted && servoTargetAngle < 0 && !isStepperRunning()) {
            step15RotationStartPosition = getStepperPosition();
            moveStepper((long)(paintRotationMotorStepsPerRevOutput * STEP15_FINAL_ROTATION_REV));
            finalRotationStarted = true;
        }
        
        // Check if 1 revolution is complete and move servo to 170°
        if (finalRotationStarted && !step15ServoMovedTo170) {
            long currentPosition = getStepperPosition();
            long positionChange = abs(currentPosition - step15RotationStartPosition);
            long oneRevSteps = paintRotationMotorStepsPerRevOutput;
            
            if (positionChange >= oneRevSteps) {
                // 1 revolution complete, move servo to 170°
                startServoMoveToAngle(STEP16_SPIN_SERVO_ANGLE_DEG);
                step15ServoMovedTo170 = true;
            }
        }
        
        // Wait for full rotation to complete
        if (finalRotationStarted && !isStepperRunning()) {
            step = 17;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 19: WAIT FOR SERVO TO REACH 170° (IF NOT ALREADY THERE)
    //! ************************************************************************
    else if (step == 17) {
        // Servo should already be moving to 170° from step 16, just wait for it
        if (servoTargetAngle < 0) {
            // Servo movement complete
            step = 18;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 20: TURN OFF PAINT GUN FOR 500MS BEFORE FINAL FRONT PASS, THEN TURN BACK ON
    //! ************************************************************************
    else if (step == 18) {
        // Turn off paint gun
        if (!step18PaintGunTurnedOff) {
            turnOffPaintGun();
            step18PaintGunTurnedOff = true;
            step18PaintGunOffTime = millis();
        }
        
        // Wait for delay period
        unsigned long elapsedTime = millis() - step18PaintGunOffTime;
        if (elapsedTime >= STEP18_PAINT_GUN_OFF_DELAY_MS) {
            // Turn paint gun back on
            if (!testModeEnabled) {
                turnOnPaintGun();
            }
            step = 19;
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 21: FINAL FACE COAT - MOVE SERVO TO 220°, WAIT 200MS, THEN HOME, THEN BACK
    //! ************************************************************************
    else if (step == 19) {
        // First move servo to initial angle (220°)
        if (!step17ServoTo190Started) {
            startServoMoveToAngle(STEP17_INITIAL_SERVO_ANGLE_DEG);
            step17ServoTo190Started = true;
        }
        // Wait for servo to reach initial angle, then wait 200ms
        else if (!step17ServoTo190Complete) {
            if (servoTargetAngle < 0) {
                // Record time when servo reached initial angle
                if (step17InitialAngleReachedTime == 0) {
                    step17InitialAngleReachedTime = millis();
                }
                
                // Wait for delay period
                unsigned long elapsedTime = millis() - step17InitialAngleReachedTime;
                if (elapsedTime >= STEP17_INITIAL_ANGLE_WAIT_MS) {
                    step17ServoTo190Complete = true;
                    // Start moving servo to home position
                    startServoMoveToAngle(SERVO_HOME_ANGLE);
                    step17ServoToHomeStarted = true;
                }
            }
        }
        // Wait for servo to reach home, then move back
        else if (!step17ServoToHomeComplete) {
            if (servoTargetAngle < 0) {
                step17ServoToHomeComplete = true;
                // Start moving servo back to 190°
                startServoMoveToAngle(STEP17_INITIAL_SERVO_ANGLE_DEG);
            }
        }
        // Wait for servo to return back
        else if (!step17ServoBackComplete) {
            if (servoTargetAngle < 0) {
                step17ServoBackComplete = true;
                step = 20;
            }
        }
        
        updateServoMovement();
        updateOTA();
        if (cycleCancelled) return;
        delay(1);
    }
    
    //! ************************************************************************
    //! STEP 22: MOVE SERVO TO HOME POSITION, THEN TURN OFF PAINT GUN
    //! ************************************************************************
    else if (step == 20) {
        // Start moving servo to home angle (non-blocking, paint gun still on)
        if (!servoToHomeStarted) {
            startServoMoveToAngle(SERVO_HOME_ANGLE);
            servoToHomeStarted = true;
        }
        
        // Wait for servo to reach home
        if (servoTargetAngle < 0) {
            // Servo has reached home, turn off paint gun
            if (!servoReachedHomeInStep17) {
                turnOffPaintGun();
                servoReachedHomeInStep17 = true;
                step = 19;
            }
        } else {
            updateServoMovement();
            updateOTA();
            if (cycleCancelled) return;
            delay(1);
        }
    }
    
    //! ************************************************************************
    //! STEP 23: TURN OFF SUCTION, CLEANUP AND RETURN TO GANTRY STATE
    //! ************************************************************************
    else if (step == 21) {
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
        backLeftWaitComplete = false;
        backLeftServoTo160Complete = false;
        backLeftServoBackComplete = false;
        backRightWaitComplete = false;
        backRightServoTo160Complete = false;
        backRightServoBackComplete = false;
        servoToHomeStarted = false;
        servoReachedHomeInStep17 = false;
        step15RotationStartPosition = 0;
        step15ServoMovedTo170 = false;
        step17ServoTo190Started = false;
        step17ServoTo190Complete = false;
        step17InitialAngleReachedTime = 0;
        step17ServoToHomeStarted = false;
        step17ServoToHomeComplete = false;
        step17ServoBackComplete = false;
        step15PaintGunTurnedOff = false;
        step15PaintGunOffTime = 0;
        step18PaintGunTurnedOff = false;
        step18PaintGunOffTime = 0;
        
        // Return to gantry state
        setMachineState(STATE_GANTRY);
    }
}
