#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Bounce2.h>

// Forward declarations
class ServoControl;
class StepperMotor;
class StorageMotor;
class HomeSwitch;

// External server instance
extern AsyncWebServer server;

// External global variables
extern bool sensorsInitialized;
extern StepperMotor* motorX;
extern StepperMotor* motorY;
extern StepperMotor* motorFork;
extern StorageMotor* motorStorage;
extern HomeSwitch* homeSwitchX;
extern HomeSwitch* homeSwitchY;
extern HomeSwitch* homeSwitchFork;
extern Bounce2::Button storagePositionSensor;
extern ServoControl* servo;
extern float currentServoAngle;
extern bool suctionState;
extern bool paintGunState;
extern bool pressurePotState;
extern float testPos1X;
extern float testPos1Y;
extern float testPos1Fork;
extern float testPos2X;
extern float testPos2Y;
extern float testPos2Fork;
extern int selectedPosition1Height;
extern int currentColumn;  // Current column position (0-5, where 0=A, 5=F)
extern int selectedColumn;  // Selected column for test cycle (0-5, where 0=A, 5=F)
extern bool testAllMode;
extern int currentTestAllHeight;
extern int testAllColumnCount;
extern int testAllStartColumn;
extern int testAllCurrentColumnIndex;
extern bool squareSensingEnabled;
extern bool testModeEnabled;
extern bool skipPaintingEnabled;
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

// External HTML content
extern const char sensors_html[] PROGMEM;

// Web Manager function declarations
extern void initializeWebServer();
extern void updateWebServer();
extern void applyMotorSettings();

// Storage functions
extern void saveTestPositions();
extern void loadTestPositions();
extern void saveMotorSettings();
extern void loadMotorSettings();
extern void saveSquareSensingState();
extern void loadSquareSensingState();
extern void saveTestModeState();
extern void loadTestModeState();
extern void saveSkipPaintingState();
extern void loadSkipPaintingState();
extern void savePaintingConfig();
extern void loadPaintingConfig();

// Hardware functions
extern void initializeSensors();
extern void initializeMotors();
extern void setServoAngle(float angle);
extern void enablePaintRotationMotor();
extern void disablePaintRotationMotor();
extern bool isPaintRotationMotorRunning();
extern bool isSquarePresent();
extern String getSensorStatesJSON();

// API Routes function
extern void setupAPIRoutes();

#endif // WEB_MANAGER_H
