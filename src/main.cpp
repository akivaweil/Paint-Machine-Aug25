#include <Arduino.h>

// OTA Manager functions (declared in OTA_Manager.cpp)
extern void initializeOTA();
extern void updateOTA();

// Web Manager functions (declared in Web_Manager.cpp)
extern void initializeWebServer();

// State functions
extern void homingState();
extern void idleState();

// State machine state
enum MachineState {
    STATE_HOMING,
    STATE_IDLE
};

MachineState currentState = STATE_HOMING;

// Forward declaration of MachineState enum
enum MachineState;

// Function to set state (accessible from states)
void setMachineState(int newState) {
    currentState = (MachineState)newState;
}

// Setup function - called once at startup
void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);

    // Initialize OTA (WiFi connection)
    initializeOTA();
    
    // Initialize Web Server (waits for WiFi connection)
    delay(2000); // Give WiFi time to connect
    initializeWebServer();
    
    // Start in homing state (will home on boot)
    currentState = STATE_HOMING;
}

// Loop function - called repeatedly
void loop() {
    // Handle OTA updates (WiFi and firmware updates) - always allow during any state
    updateOTA();

    // Run current state
    switch (currentState) {
        case STATE_HOMING:
            homingState();
            break;
        case STATE_IDLE:
            idleState();
            break;
    }

    // Small delay to prevent overwhelming the processor
    delay(10);
}
