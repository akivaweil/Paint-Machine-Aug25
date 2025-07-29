#include <Arduino.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"
#include "StateMachine/STATES/00_IDLE.h"
#include "StateMachine/STATES/01_HOMING.h"
#include "StateMachine/FUNCTIONS/StepperMotor.h"
#include "StateMachine/FUNCTIONS/HomeSwitch.h"

// OTA Manager functions
void initializeOTA();
void updateOTA();
bool isOTAReady();
String getOTAIpAddress();
bool isWiFiConnected();

//* ************************************************************************
//* ************************ MAIN APPLICATION *******************************
//* ************************************************************************

// Global motor objects
StepperMotor* x1Motor = nullptr;
StepperMotor* x2Motor = nullptr;
StepperMotor* yMotor = nullptr;
StepperMotor* forkMotor = nullptr;

// Global home switch objects
HomeSwitch* x1HomeSwitch = nullptr;
HomeSwitch* x2HomeSwitch = nullptr;
HomeSwitch* yHomeSwitch = nullptr;
HomeSwitch* forkHomeSwitch = nullptr;

// State machine variables
int currentState = 0;
int nextState = 0;
bool stateInitialized = false;

// Function pointers for state functions
typedef void (*StateInitFunc)();
typedef void (*StateRunFunc)();
typedef bool (*StateTransitionFunc)();
typedef int (*StateNextFunc)();
typedef void (*StateCleanupFunc)();

// State function arrays
StateInitFunc stateInitFunctions[] = {
    IdleState::initialize,
    HomingState::initialize
};

StateRunFunc stateRunFunctions[] = {
    IdleState::run,
    HomingState::run
};

StateTransitionFunc stateTransitionFunctions[] = {
    IdleState::shouldTransition,
    HomingState::shouldTransition
};

StateNextFunc stateNextFunctions[] = {
    IdleState::getNextState,
    HomingState::getNextState
};

StateCleanupFunc stateCleanupFunctions[] = {
    IdleState::cleanup,
    HomingState::cleanup
};

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Initialize input pins
    pinMode(START_BUTTON_PIN, INPUT_PULLDOWN);
    pinMode(STOP_BUTTON_PIN, INPUT_PULLDOWN);
    pinMode(E_STOP_PIN, INPUT_PULLDOWN);
    pinMode(RESET_PIN, INPUT_PULLDOWN);
    pinMode(PAUSE_PIN, INPUT_PULLDOWN);
    
    // Initialize status LEDs
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(ERROR_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
    digitalWrite(ERROR_LED_PIN, LOW);
    
    // Create motor objects
    x1Motor = new StepperMotor(X1_STEP_PIN, X1_DIR_PIN, X1_ENABLE_PIN, X1_HOME_PIN, X1_LIMIT_PIN, "X1");
    x2Motor = new StepperMotor(X2_STEP_PIN, X2_DIR_PIN, X2_ENABLE_PIN, X2_HOME_PIN, X2_LIMIT_PIN, "X2");
    yMotor = new StepperMotor(Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN, Y_HOME_PIN, Y_LIMIT_PIN, "Y");
    forkMotor = new StepperMotor(FORK_STEP_PIN, FORK_DIR_PIN, FORK_ENABLE_PIN, FORK_HOME_PIN, FORK_LIMIT_PIN, "Fork");
    
    // Initialize motors
    x1Motor->initialize();
    x2Motor->initialize();
    yMotor->initialize();
    forkMotor->initialize();
    
    // Create home switch objects
    x1HomeSwitch = new HomeSwitch(X1_HOME_PIN, "X1 Home");
    x2HomeSwitch = new HomeSwitch(X2_HOME_PIN, "X2 Home");
    yHomeSwitch = new HomeSwitch(Y_HOME_PIN, "Y Home");
    forkHomeSwitch = new HomeSwitch(FORK_HOME_PIN, "Fork Home");
    
    // Initialize home switches
    x1HomeSwitch->initialize();
    x2HomeSwitch->initialize();
    yHomeSwitch->initialize();
    forkHomeSwitch->initialize();
    
    // Start in idle state
    currentState = 0;
    nextState = 0;
    stateInitialized = false;
    
    // Initialize OTA
    initializeOTA();
    
    // Initial status indication
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(1000);
    digitalWrite(STATUS_LED_PIN, LOW);
}

void loop() {
    // Update OTA
    updateOTA();
    
    // Update home switches
    if (x1HomeSwitch) x1HomeSwitch->update();
    if (x2HomeSwitch) x2HomeSwitch->update();
    if (yHomeSwitch) yHomeSwitch->update();
    if (forkHomeSwitch) forkHomeSwitch->update();
    
    // State machine logic
    if (!stateInitialized) {
        // Initialize current state
        if (currentState < sizeof(stateInitFunctions) / sizeof(stateInitFunctions[0])) {
            stateInitFunctions[currentState]();
        }
        stateInitialized = true;
    }
    
    // Run current state
    if (currentState < sizeof(stateRunFunctions) / sizeof(stateRunFunctions[0])) {
        stateRunFunctions[currentState]();
    }
    
    // Check for state transition
    if (currentState < sizeof(stateTransitionFunctions) / sizeof(stateTransitionFunctions[0])) {
        if (stateTransitionFunctions[currentState]()) {
            // Get next state
            if (currentState < sizeof(stateNextFunctions) / sizeof(stateNextFunctions[0])) {
                nextState = stateNextFunctions[currentState]();
            }
            
            // Cleanup current state
            if (currentState < sizeof(stateCleanupFunctions) / sizeof(stateCleanupFunctions[0])) {
                stateCleanupFunctions[currentState]();
            }
            
            // Transition to next state
            currentState = nextState;
            stateInitialized = false;
        }
    }
    
    // Small delay to prevent overwhelming the system
    delay(1);
}