#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Bounce2.h>

// Forward declarations
class ServoControl;
class StepperMotor;
class HomeSwitch;

// External server instance
extern AsyncWebServer server;

// External global variables
extern bool sensorsInitialized;
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;
extern StepperMotor* motorStorage;
extern StepperMotor* motorPaintRotation;
extern HomeSwitch* homeSwitchX;
extern HomeSwitch* homeSwitchY;
extern HomeSwitch* homeSwitchFork;
extern Bounce2::Button storagePositionSensor;
extern ServoControl* servo;
extern float currentServoAngle;
extern bool suctionState;
extern bool paintGunState;
extern float runPos1X;
extern float runPos1Y;
extern float runPos1Fork;
extern float runPos2X;
extern float runPos2Y;
extern float runPos2Fork;
extern int selectedPosition1Height;
extern int currentColumn;  // Current column position (0-5, where 0=A, 5=F)
extern int selectedColumn;  // Selected column for run cycle (0-5, where 0=A, 5=F)
extern bool runAllMode;
extern int currentRunAllHeight;
extern int runAllColumnCount;
extern int runAllStartColumn;
extern int runAllCurrentColumnIndex;
extern bool squareSensingEnabled;
extern long motorSpeedX;
extern long motorSpeedY;
extern long motorSpeedFork;
extern long motorAccelX;
extern long motorAccelY;
extern long motorAccelFork;
extern long motorSpeedPaintRotation;
extern long motorAccelPaintRotation;
extern long motorSpeedStorage;
extern long motorAccelStorage;
extern long storageMotorStepsPerClick;
extern long storageMotorTrimDistance;
extern long paintRotationMotorStepsPerClick;
extern long paintRotationMotorStepsPerRevOutput;
extern float servoSpeed;
extern bool cyclePaused;
extern bool cycleCancelled;

// External HTML content
extern const char sensors_html[] PROGMEM;

// Web Manager function declarations
extern void initializeWebServer();
extern void updateWebServer();
extern void applyMotorSettings();

// Storage functions
extern void saveRunPositions();
extern void loadRunPositions();
extern void saveMotorSettings();
extern void loadMotorSettings();
extern void saveSquareSensingState();
extern void loadSquareSensingState();

// Hardware functions
extern void initializeSensors();
extern void initializeMotors();
extern void setServoAngle(float angle);
extern void enablePaintRotationMotor();
extern void disablePaintRotationMotor();
extern bool isPaintRotationMotorRunning();
extern String getSensorStatesJSON();

// API Routes function
extern void setupAPIRoutes();

#endif // WEB_MANAGER_H
