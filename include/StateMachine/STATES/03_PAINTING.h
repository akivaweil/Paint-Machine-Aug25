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

// Painting configuration variables
extern float cfgServoHomeAngle;
extern float cfgServoPaintingAngle;
extern unsigned long cfgServoUpdateIntervalMs;
extern float cfgServoTargetReachedThresholdDeg;
extern float cfgServoFarFromTargetThresholdDeg;
extern float cfgServoMinMovementDeg;
extern float cfgServoMaxStepSizeDeg;
extern float cfgStep1WaitingPositionXOffsetInches;
extern float cfgStep1WaitingPositionYOffsetInches;
extern unsigned long cfgStep2WaitingPositionDelayMs;
extern float cfgStep3ServoFastSpeed;
extern unsigned long cfgStep4InitialRotationDelayMs;
extern float cfgStep4InitialRotationRev;
extern float cfgStep5LeftRotationRev;
extern unsigned long cfgStep6LeftSideWaitMs;
extern float cfgStep7BackLeftRotationRev;
extern unsigned long cfgStep8BackLeftWaitMs;
extern float cfgStep8ServoBackAngleDeg;
extern float cfgStep8BackLeftServoSpeed;
extern unsigned long cfgStep8BackLeftPaintDelayMs;
extern float cfgStep9BackRotationRev;
extern unsigned long cfgStep10BackSideWaitMs;
extern float cfgStep10ServoBackAngleDeg;
extern float cfgStep10BackServoSpeed;
extern unsigned long cfgStep10BackPaintDelayMs;
extern float cfgStep11BackRightRotationRev;
extern unsigned long cfgStep12BackRightWaitMs;
extern float cfgStep12ServoBackAngleDeg;
extern float cfgStep12BackRightServoSpeed;
extern unsigned long cfgStep12BackRightPaintDelayMs;
extern float cfgStep13RightRotationRev;
extern unsigned long cfgStep14RightSideWaitMs;
extern float cfgStep14ServoRightAngleDeg;
extern float cfgStep14RightServoSpeed;
extern unsigned long cfgStep14RightPaintDelayMs;
extern unsigned long cfgStep15PaintGunOffDelayMs;
extern float cfgStep15FinalRotationRev;
extern float cfgStep15FirstRevServoAngleDeg;
extern float cfgStep16SpinServoAngleDeg;
extern unsigned long cfgStep18PaintGunOffDelayMs;
extern float cfgStep17InitialServoAngleDeg;
extern unsigned long cfgStep17InitialAngleWaitMs;
extern float cfgStep17ServoSpeed;

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

