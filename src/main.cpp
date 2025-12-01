#include <Arduino.h>

// OTA Manager functions (declared in OTA_Manager.cpp)
extern void initializeOTA();
extern void updateOTA();

// Web Manager functions (declared in Web_Manager.cpp)
extern void initializeWebServer();

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
}

// Loop function - called repeatedly
void loop() {
    // Handle OTA updates (WiFi and firmware updates)
    updateOTA();

    // Small delay to prevent overwhelming the processor
    delay(10);
}
