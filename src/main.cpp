#include <Arduino.h>

// OTA Manager functions (declared in OTA_Manager.cpp)
extern void initializeOTA();
extern void updateOTA();

// Web Manager functions (declared in Web_Manager.cpp)
extern void initializeWebServer();
extern void updateWebServer();

// State functions
extern void homingState();
extern void idleState();
extern void pickPlaceState();
extern void paintingState();

// State machine state
enum MachineState {
    STATE_HOMING,
    STATE_IDLE,
    STATE_PICK_PLACE,
    STATE_PAINTING
};

MachineState currentState = STATE_HOMING;

// Forward declaration of MachineState enum
enum MachineState;

// Function to set state (accessible from states)
void setMachineState(int newState) {
    currentState = (MachineState)newState;
}

// Function to get current state (accessible from web manager)
int getMachineState() {
    return (int)currentState;
}

// Setup function - called once at startup
void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);

    // Initialize OTA (WiFi connection - non-blocking)
    initializeOTA();
    
    // Initialize Web Server (doesn't need WiFi to be connected)
    initializeWebServer();
    
    // Start in homing state (will home on boot)
    currentState = STATE_HOMING;
}

// Loop function - called repeatedly
void loop() {
    // Handle OTA updates (WiFi and firmware updates) - always allow during any state
    updateOTA();
    
    // Update web server (handles motor enable/disable checks)
    updateWebServer();

    // Run current state
    switch (currentState) {
        case STATE_HOMING:
            homingState();
            break;
        case STATE_IDLE:
            idleState();
            break;
        case STATE_PICK_PLACE:
            pickPlaceState();
            break;
        case STATE_PAINTING:
            paintingState();
            break;
    }

    // Small delay to prevent overwhelming the processor
    delay(10);
}
