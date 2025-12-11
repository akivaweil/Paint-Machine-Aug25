#ifndef PAINTING_STATE_H
#define PAINTING_STATE_H

#include <Arduino.h>

//* ************************************************************************
//* ************************ PAINTING STATE ********************************
//* ************************************************************************

// State machine function
extern void setMachineState(int state);
#define STATE_PICK_PLACE 2

// External motor instances (defined in Web_Manager.cpp)
class StepperMotor;
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;

// External servo and paint gun controls
class ServoControl;
extern ServoControl* servo;
extern float currentServoAngle;
extern float servoSpeed;
extern void enablePaintRotationMotor();
extern void disablePaintRotationMotor();
extern void setServoAngle(float angle);

// OTA Manager function
extern void updateOTA();

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

// Paint motor controller functions
extern void setStepperSpeed(long speed);
extern void setStepperAcceleration(long accel);
extern void resetStepperPosition();
extern long getStepperPosition();
extern void moveStepper(long steps);
extern bool isStepperRunning();
extern void stopStepper();

// Y-axis motor speed
extern long motorSpeedY;

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ ⚔️ HELPER FUNCTIONS                                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝

// Servo movement functions
void updateServoMovement();
void startServoMoveToAngle(float angle);

// Motor control functions
void waitForMotor(StepperMotor* motor);
void waitForMotors(StepperMotor* motor1, StepperMotor* motor2);
void startMoveToWaitingPosition();

// Paint gun and suction control
void turnOnPaintGun();
void turnOffPaintGun();
void turnOnSuction();
void turnOffSuction();

// Main state function
void paintingState();

#endif // PAINTING_STATE_H

