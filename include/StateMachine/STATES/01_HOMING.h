#ifndef HOMING_STATE_H
#define HOMING_STATE_H

#include "../FUNCTIONS/StepperMotor.h"
#include "../FUNCTIONS/HomeSwitch.h"
#include "../../../src/config/Config.h"
#include "../../../src/config/Homing_Config.h"

// Homing sequence phases
enum HomingPhase {
    HOMING_INIT,
    HOMING_FORK,
    HOMING_X_AND_Y,
    HOMING_MOVE_AWAY,
    HOMING_COMPLETE,
    HOMING_ERROR
};

class HomingState {
private:
    // Motor objects
    StepperMotor* motorX;
    StepperMotor* motorY;
    StepperMotor* motorFork;

    // Home switch object
    HomeSwitch* homeSwitches;

    // State variables
    HomingPhase currentPhase;
    unsigned long phaseStartTime;
    bool phaseTimeout;

    // Homing progress flags
    bool forkHomed;
    bool xHomed;
    bool yHomed;
    
    // Move away tracking
    long moveAwayStartPosition;
    long moveAwayTargetSteps;

    // Helper functions
    void startHomingSequence();
    void homeFork();
    void homeXAndY();
    void moveAwayFromHome();
    bool checkPhaseTimeout();

public:
    // Constructor/Destructor
    HomingState();
    ~HomingState();

    // State interface functions
    void enter();
    void update();
    void exit();

    // Status functions
    bool isComplete();
    bool hasError();
    HomingPhase getCurrentPhase();

    // Emergency functions
    void emergencyStop();
};

#endif // HOMING_STATE_H
