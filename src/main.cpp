#include <Arduino.h>

// OTA Manager functions (declared in OTA_Manager.cpp)
extern void initializeOTA();
extern void updateOTA();

// Setup function - called once at startup
void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);

    // Initialize OTA (WiFi connection)
    initializeOTA();
}

// Loop function - called repeatedly
void loop() {
    // Handle OTA updates (WiFi and firmware updates)
    updateOTA();

    // Small delay to prevent overwhelming the processor
    delay(10);
}
