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
extern void setServoAngle(float angle);

// OTA Manager function
extern void updateOTA();

// State machine function
extern void setMachineState(int state);
#define STATE_GANTRY 2

// Cycle control flags
extern bool cyclePaused;
extern bool cycleCancelled;

// Position values (set from web interface)
extern float pos2X;
extern float pos2Y;
extern float pos2Fork;

// Paint rotation motor steps per revolution
extern long paintRotationMotorStepsPerRevOutput;

//* ************************************************************************
//* ************************ PAINTING STATE ********************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ HELPER FUNCTIONS                                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Wait for a motor to finish moving
void waitForMotor(StepperMotor* motor) {
    while (motor && motor->isMotorRunning()) {
        updateOTA();
        if (cycleCancelled) return;
        delay(1);
    }
}

// Wait for multiple motors to finish moving
void waitForMotors(StepperMotor* motor1, StepperMotor* motor2) {
    while ((motor1 && motor1->isMotorRunning()) || (motor2 && motor2->isMotorRunning())) {
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
        motorFork->moveInches(pos2Fork);
        waitForMotor(motorFork);
    }
}

// Start paint rotation motor for 2 full revolutions and return start position
long startPaintRotationTwoRevolutions() {
    enablePaintRotationMotor();
    long startPos = 0;
    if (motorPaintRotation) {
        startPos = motorPaintRotation->getCurrentPosition();
        long steps = paintRotationMotorStepsPerRevOutput * 2;
        motorPaintRotation->moveSteps(steps);
    }
    return startPos;
}

// Move to waiting position (5 inches right of position 3)
void moveToWaitingPosition() {
    if (!motorX || !motorY) return;
    
    // Get current positions
    float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
    float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
    
    // Calculate target position (5 inches right of pos3)
    // Position 3 is: X = -pos2X, Y = -pos2Y + 0.5
    float targetX = -pos2X + 5.0;
    float targetY = -pos2Y + 0.5;
    
    // Calculate movement needed
    float moveX = targetX - currentX;
    float moveY = targetY - currentY;
    
    // Move both motors simultaneously
    motorX->moveInches(moveX);
    motorY->moveInches(moveY);
    
    // Wait for both to finish
    waitForMotors(motorX, motorY);
}

// Move servo to angle (blocking)
void moveServoToAngle(float angle) {
    setServoAngle(angle);
}

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ PAINTING STATE                                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

void paintingState() {
    static int step = 0;
    static bool paintingStarted = false;
    static long paintMotorStartPosition = 0;
    static bool servoMoving = false;
    static float servoTargetAngle = 0.0;
    static unsigned long lastServoUpdate = 0;
    
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
        servoMoving = false;
        
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
    }
    
    //! ************************************************************************
    //! STEP 1: RETRACT FORK TO POSITION 2
    //! ************************************************************************
    if (step == 0) {
        retractForkToPosition2();
        if (cycleCancelled) return;
        step = 1;
    }
    
    //! ************************************************************************
    //! STEP 2: MOVE (GANTRY) TO WAITING POSITION WHILE TURNING ON PAINT GUN AND SUCTION
    //! ************************************************************************
    else if (step == 1) {
        // Start gantry movement
        if (motorX && motorY) {
            // Get current positions
            float currentX = motorX->stepsToInches(motorX->getCurrentPosition());
            float currentY = motorY->stepsToInches(motorY->getCurrentPosition());
            
            // Calculate target position (5 inches right of pos3)
            // Position 3 is: X = -pos2X, Y = -pos2Y + 0.5
            float targetX = -pos2X + 5.0;
            float targetY = -pos2Y + 0.5;
            
            // Calculate movement needed
            float moveX = targetX - currentX;
            float moveY = targetY - currentY;
            
            // Start both motors moving simultaneously
            motorX->moveInches(moveX);
            motorY->moveInches(moveY);
        }
        
        // Turn on paint gun and suction (while gantry moves)
        turnOnPaintGun();
        turnOnSuction();
        
        // Wait for gantry to finish moving
        waitForMotors(motorX, motorY);
        if (cycleCancelled) return;
        
        step = 2;
    }
    
    //! ************************************************************************
    //! STEP 3: START PAINT ROTATION MOTOR FOR 2 REVOLUTIONS AND START SERVO MOVING TO 220 DEGREES (SIMULTANEOUSLY)
    //! ************************************************************************
    else if (step == 2) {
        // Start paint rotation motor (non-blocking)
        paintMotorStartPosition = startPaintRotationTwoRevolutions();
        
        // Start servo moving to 220 degrees (non-blocking gradual movement)
        if (servo) {
            servoTargetAngle = 220.0;
            servoMoving = true;
            lastServoUpdate = millis();
        }
        
        step = 3;
    }
    
    // Non-blocking gradual servo movement (runs continuously while servoMoving is true)
    if (servoMoving && servo) {
        unsigned long currentTime = millis();
        const float stepSize = 0.5;  // Step size in degrees
        const float stepDelayMs = (stepSize / servoSpeed) * 1000.0;  // Delay in milliseconds
        
        if (currentTime - lastServoUpdate >= (unsigned long)stepDelayMs) {
            float diff = servoTargetAngle - currentServoAngle;
            
            if (abs(diff) > stepSize) {
                // Move one step closer to target
                float increment = (diff > 0) ? stepSize : -stepSize;
                currentServoAngle += increment;
                servo->write(currentServoAngle);
                lastServoUpdate = currentTime;
            } else {
                // Reached target
                currentServoAngle = servoTargetAngle;
                servo->write(currentServoAngle);
                servoMoving = false;
            }
        }
    }
    
    //! ************************************************************************
    //! STEP 4: WAIT FOR PAINT MOTOR TO COMPLETE 1 REVOLUTION, THEN MOVE SERVO BACK TO HOME POSITION
    //! ************************************************************************
    else if (step == 3) {
        // Wait for 1 revolution from start
        waitForPaintRotationRevolutions(paintMotorStartPosition, 1.0);
        if (cycleCancelled) return;
        
        // Start servo moving back to home position (non-blocking gradual movement)
        if (servo) {
            servoTargetAngle = SERVO_HOME_ANGLE;
            servoMoving = true;
            lastServoUpdate = millis();
        }
        
        step = 4;
    }
    
    //! ************************************************************************
    //! STEP 5: WAIT FOR PAINT MOTOR TO FINISH, THEN TURN OFF EVERYTHING
    //! ************************************************************************
    else if (step == 4) {
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
        
        // Return to gantry state
        setMachineState(STATE_GANTRY);
    }
}
