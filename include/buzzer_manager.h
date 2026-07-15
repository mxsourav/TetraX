#pragma once
#include <Arduino.h>
#include "app_config.h"

class BuzzerManager {
public:
    static void init() {
        pinMode(AppConfig::BUZZER_PIN, OUTPUT);
        digitalWrite(AppConfig::BUZZER_PIN, LOW); // OFF
    }

    static void beepSync(uint16_t durationMs = 5) {
        if (durationMs > 5) durationMs = 5; // Hard cap
        digitalWrite(AppConfig::BUZZER_PIN, HIGH);
        delay(durationMs);
        digitalWrite(AppConfig::BUZZER_PIN, LOW);
    }
    
    static void beep(uint16_t durationMs = 5, uint8_t count = 1) {
        (void)count;
        if (durationMs > 5) durationMs = 5; // Hard cap at 5ms
        // 5ms is so short that we can safely block. This guarantees
        // the buzzer always turns off, even if a delay() or while() 
        // loop immediately follows the button press in other modules.
        digitalWrite(AppConfig::BUZZER_PIN, HIGH);
        delay(durationMs);
        digitalWrite(AppConfig::BUZZER_PIN, LOW);
    }
    
    static void update() {
        // No-op. The buzzer is guaranteed to be off when beep() returns.
    }
};
