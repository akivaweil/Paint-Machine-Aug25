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

// Function to print IP address with formatting
void printIPAddress();

void initializeOTA() {
    if (otaInitialized) {
        return;
    }
    
    // Connect to WiFi (Non-blocking)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // We don't wait here anymore to allow faster startup
    // OTA will be initialized in updateOTA() once connected
}

void printIPAddress() {
    Serial.println("=== WIFI CONNECTED ===");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("======================");
}

void setupArduinoOTA() {
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
    Serial.println("OTA Initialized");
    printIPAddress();
}

void updateOTA() {
    // Check if we need to initialize OTA (first connection)
    if (!otaInitialized && WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        setupArduinoOTA();
    }

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
            // If we just reconnected, print IP address
            if (!wifiConnected) {
                printIPAddress();
            }
            wifiConnected = true;
            // If we reconnected but OTA wasn't initialized (shouldn't happen if logic above is correct, but safety)
            if (!otaInitialized) {
                setupArduinoOTA();
            }
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
