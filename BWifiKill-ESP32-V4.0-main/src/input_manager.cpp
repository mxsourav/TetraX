#include "input_manager.h"
#undef digitalRead

InputManager Input;

const uint8_t InputManager::PINS[BTN_COUNT] = {
    AppConfig::BTN_UP,
    AppConfig::BTN_DOWN,
    255, // BTN_LEFT (Virtual)
    255, // BTN_RIGHT (Virtual)
    AppConfig::BTN_OK,
    AppConfig::BTN_BACK,
    AppConfig::BTN_AUX
};

void InputManager::begin() {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        if (PINS[i] != 255) {
            pinMode(PINS[i], INPUT_PULLUP);
        }
        stableState[i]  = false;
        rawPrev[i]      = false;
        lastEdgeMs[i]   = 0;
        pressStartMs[i] = 0;
        lastRepeatMs[i] = 0;
        longFired[i]    = false;
        events[i]       = EVT_NONE;
    }
}

void InputManager::update() {
    const uint32_t now = millis();

    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        events[i] = EVT_NONE;

        bool raw = virtualState[i];
        
        if (PINS[i] != 255) {
            raw |= (::digitalRead(PINS[i]) == LOW);
        }

        if (raw != rawPrev[i]) {
            lastEdgeMs[i] = now;
            rawPrev[i]    = raw;
        }

        const bool stableElapsed = (now - lastEdgeMs[i]) >= AppConfig::INPUT_DEBOUNCE_MS;

        if (stableElapsed && raw != stableState[i]) {
            stableState[i] = raw;

            if (raw) {
                events[i] = EVT_PRESSED;
                pressStartMs[i] = now;
                lastRepeatMs[i] = now;
                longFired[i]    = false;
                

            } else {
                events[i] = EVT_RELEASED;
                
                if (i == BTN_ID_BACK) {
                    if (!comboUsed) {
                        Serial.println("[MODIFIER] BACK released without combo. Triggering CANCEL.");
                        cancelFlag = true;
                        events[BTN_ID_BACK] = EVT_PRESSED;
                    } else {
                        Serial.println("[MODIFIER] BACK released WITH combo. Suppressed.");
                    }
                    comboUsed = false;
                }
            }
            continue;
        }

        if (stableState[i]) {
            const uint32_t holdMs = now - pressStartMs[i];

            if (!longFired[i] && holdMs >= AppConfig::INPUT_LONG_PRESS_MS) {
                // Ignore long press for virtual BACK since we don't emit PRESS for it
                if (i != BTN_ID_BACK) {
                    events[i]    = EVT_LONG_PRESS;
                    longFired[i] = true;
                }
                continue;
            }

            if (holdMs >= AppConfig::INPUT_REPEAT_DELAY_MS &&
                (now - lastRepeatMs[i]) >= AppConfig::INPUT_REPEAT_RATE_MS) {
                // Ignore repeat for virtual BACK
                if (i != BTN_ID_BACK) {
                    events[i]       = EVT_REPEAT;
                    lastRepeatMs[i] = now;
                }
            }
        }
    }
}

uint32_t InputManager::heldMs(ButtonId id) const {
    if (!stableState[id]) return 0;
    return millis() - pressStartMs[id];
}

void InputManager::consume(ButtonId id) {
    if (id >= BTN_COUNT) return;
    events[id] = EVT_NONE;
}

void InputManager::resetAll() {
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        events[i] = EVT_NONE;
        stableState[i] = false;
        longFired[i] = false;
        virtualState[i] = false;
    }
    cancelFlag = false;
}

void InputManager::injectVirtualState(ButtonId id, bool pressed) {
    if (id < BTN_COUNT) {
        virtualState[id] = pressed;
    }
}

bool InputManager::cancelRequested() {
    update();
    if (cancelFlag) {
        cancelFlag = false;
        Serial.println("[CANCEL] Requested via InputManager.");
        return true;
    }
    return false;
}

int InputManager::hijackedDigitalRead(uint8_t pin) {
    update(); // Feed the state machine

    // Only intercept known buttons to avoid breaking SD card or SPI logic
    if (pin != AppConfig::BTN_UP && pin != AppConfig::BTN_DOWN && 
        pin != AppConfig::BTN_OK && pin != AppConfig::BTN_BACK && 
        pin != AppConfig::BTN_AUX && pin != 25) {
        return ::digitalRead(pin);
    }

    if (pin == AppConfig::BTN_UP) {
        return held(BTN_ID_UP) ? LOW : HIGH;
    }
    if (pin == AppConfig::BTN_DOWN) {
        return held(BTN_ID_DOWN) ? LOW : HIGH;
    }
    if (pin == AppConfig::BTN_OK) {
        return held(BTN_ID_OK) ? LOW : HIGH;
    }
    if (pin == AppConfig::BTN_AUX) {
        return held(BTN_ID_AUX) ? LOW : HIGH;
    }
    if (pin == AppConfig::BTN_BACK || pin == 25) {
        if (cancelFlag) {
            cancelFlag = false;
            Serial.println("[CANCEL] Faking LOW on hijacked pin 25 to break loop.");
            return LOW;
        }
        return HIGH;
    }
    return ::digitalRead(pin);
}