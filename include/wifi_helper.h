#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

class WifiHelper {
public:
    /**
     * @brief Safely initializes the ESP32 WiFi interface for promiscuous packet injection.
     * Guaranteed to tear down any stale/invalid driver state before initializing.
     */
    static void setupPromiscuous() {
        // 1. Force a clean teardown of the current WiFi state.
        // We do NOT use WiFi.disconnect(true) because the asynchronous disconnect
        // events can interfere with the immediate driver restart.
        esp_wifi_set_promiscuous(false);
        WiFi.mode(WIFI_OFF);
        delay(50); // Allow hardware MAC to settle
        
        // 2. Let Arduino cleanly initialize the STA driver context.
        WiFi.mode(WIFI_STA);
        
        // 3. Wait for the driver state machine to enter STA mode.
        delay(50);
        
        // 4. Overlay promiscuous mode on top of the valid STA interface.
        esp_wifi_set_promiscuous(true);
    }

    /**
     * @brief Gracefully disables promiscuous mode and shuts down the WiFi radio.
     */
    static void teardownPromiscuous() {
        esp_wifi_set_promiscuous(false);
        WiFi.mode(WIFI_OFF);
        delay(50);
    }
};
