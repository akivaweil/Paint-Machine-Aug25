#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>

// Forward declarations
class ServoControl;
class StepperMotor;

// Web Manager function declarations (implemented in Web_Manager.cpp)
extern void initializeWebServer();
extern void updateWebServer();
extern void applyMotorSettings();
extern void enablePaintRotationMotor();
extern void disablePaintRotationMotor();
extern bool isPaintRotationMotorRunning();

// External instances
extern ServoControl* servo;
extern float currentServoAngle;
extern float servoSpeed;
extern StepperMotor* motorPaintRotation;

#endif // WEB_MANAGER_H


