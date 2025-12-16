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

// Painting configuration settings (runtime adjustable)
extern float paintingConfigServoHomeAngle;
extern float paintingConfigServoPaintingAngle;
extern unsigned long paintingConfigServoUpdateIntervalMs;
extern float paintingConfigServoTargetReachedThresholdDeg;
extern float paintingConfigServoFarFromTargetThresholdDeg;
extern float paintingConfigServoMinMovementDeg;
extern float paintingConfigServoMaxStepSizeDeg;
extern float paintingConfigStep1WaitingPositionXOffsetInches;
extern float paintingConfigStep1WaitingPositionYOffsetInches;
extern unsigned long paintingConfigStep2WaitingPositionDelayMs;
extern float paintingConfigStep3ServoFastSpeed;
extern unsigned long paintingConfigStep4InitialRotationDelayMs;
extern float paintingConfigStep4InitialRotationRev;
extern float paintingConfigStep5LeftRotationRev;
extern unsigned long paintingConfigStep6LeftSideWaitMs;
extern float paintingConfigStep7BackLeftRotationRev;
extern unsigned long paintingConfigStep8BackLeftWaitMs;
extern float paintingConfigStep8ServoBackAngleDeg;
extern float paintingConfigStep8BackLeftServoSpeed;
extern unsigned long paintingConfigStep8BackLeftPaintDelayMs;
extern float paintingConfigStep9BackRotationRev;
extern unsigned long paintingConfigStep10BackSideWaitMs;
extern float paintingConfigStep10ServoBackAngleDeg;
extern float paintingConfigStep10BackServoSpeed;
extern unsigned long paintingConfigStep10BackPaintDelayMs;
extern float paintingConfigStep11BackRightRotationRev;
extern unsigned long paintingConfigStep12BackRightWaitMs;
extern float paintingConfigStep12ServoBackAngleDeg;
extern float paintingConfigStep12BackRightServoSpeed;
extern unsigned long paintingConfigStep12BackRightPaintDelayMs;
extern float paintingConfigStep13RightRotationRev;
extern unsigned long paintingConfigStep14RightSideWaitMs;
extern float paintingConfigStep14ServoRightAngleDeg;
extern float paintingConfigStep14RightServoSpeed;
extern unsigned long paintingConfigStep14RightPaintDelayMs;
extern unsigned long paintingConfigStep15PaintGunOffDelayMs;
extern float paintingConfigStep15FinalRotationRev;
extern float paintingConfigStep15FirstRevServoAngleDeg;
extern float paintingConfigStep16SpinServoAngleDeg;
extern unsigned long paintingConfigStep18PaintGunOffDelayMs;
extern float paintingConfigStep17InitialServoAngleDeg;
extern unsigned long paintingConfigStep17InitialAngleWaitMs;
extern float paintingConfigStep17ServoSpeed;

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
