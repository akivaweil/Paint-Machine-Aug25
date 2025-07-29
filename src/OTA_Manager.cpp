#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "config/Config.h"
#include "config/Pin_Definitions.h"

//* ************************************************************************
//* ************************ OTA MANAGER ***********************************
//* ************************************************************************

// OTA status
bool otaInitialized = false;

// WiFi connection status
bool wifiConnected = false;
unsigned long lastWifiCheck = 0;
const unsigned long WIFI_CHECK_INTERVAL = 30000; // Check every 30 seconds

void initializeOTA() {
    if (otaInitialized) {
        return;
    }
    
    // Connect to WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        
        // Configure ArduinoOTA
        ArduinoOTA.setHostname("PaintMachine");
        ArduinoOTA.setPassword("paint123");
        
        ArduinoOTA.onStart([]() {
            // Turn off status LED during update
            digitalWrite(STATUS_LED_PIN, LOW);
        });
        
        ArduinoOTA.onEnd([]() {
            // Turn on status LED when update complete
            digitalWrite(STATUS_LED_PIN, HIGH);
        });
        
        ArduinoOTA.onError([](ota_error_t error) {
            // Blink error LED on update error
            digitalWrite(ERROR_LED_PIN, HIGH);
        });
        
        // Start OTA
        ArduinoOTA.begin();
        
        otaInitialized = true;
    }
}

void updateOTA() {
    // Handle OTA updates
    if (otaInitialized) {
        ArduinoOTA.handle();
    }
    
    // Check WiFi connection periodically
    unsigned long currentTime = millis();
    if (currentTime - lastWifiCheck >= WIFI_CHECK_INTERVAL) {
        if (WiFi.status() != WL_CONNECTED) {
            wifiConnected = false;
            // Try to reconnect
            WiFi.reconnect();
        } else {
            wifiConnected = true;
        }
        lastWifiCheck = currentTime;
    }
}

bool isOTAReady() {
    return otaInitialized && wifiConnected;
}

String getOTAIpAddress() {
    if (wifiConnected) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

bool isWiFiConnected() {
    return wifiConnected;
} 