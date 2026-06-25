#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>
#include "app_config.h"

// =============================================================
// InputManager
// -------------------------------------------------------------
// Reemplaza el patrón:
//     if (digitalRead(BTN_UP) == LOW) { ...; delay(200); }
// por una API basada en eventos, sin delay() y con debounce real.
//
// USO TÍPICO en cada loop de app:
//
//   Input.update();                              // una vez por frame
//   if (Input.pressed(BTN_ID_OK))   { ... }
//   if (Input.repeating(BTN_ID_UP)) { menu_idx--; }
//   if (Input.longPressed(BTN_ID_BACK)) { hardExit(); }
//
// "repeating" = primer pulso + auto-repeat sostenido (ideal menús).
// "pressed"   = solo el flanco de bajada (ideal confirmaciones).
// "longPressed" = se dispara UNA sola vez al cruzar el umbral.
//
// CONSUME:
//   Cuando un módulo con sub-vistas (ej. wifiscan en detalle)
//   maneja un evento y NO quiere que main.cpp también lo procese,
//   llama Input.consume(BTN_ID_BACK) para marcarlo como atendido
//   en este frame. La siguiente call a Input.update() lo limpia.
// =============================================================

enum ButtonId : uint8_t {
    BTN_ID_UP = 0,
    BTN_ID_DOWN,
    BTN_ID_LEFT,
    BTN_ID_RIGHT,
    BTN_ID_OK,
    BTN_ID_BACK,
    BTN_ID_AUX,
    BTN_COUNT
};

enum ButtonEvent : uint8_t {
    EVT_NONE = 0,
    EVT_PRESSED,      // Flanco: acaba de pulsarse
    EVT_RELEASED,     // Flanco: acaba de soltarse
    EVT_REPEAT,       // Auto-repeat mientras se mantiene
    EVT_LONG_PRESS    // Sostén pasó el umbral (dispara una sola vez)
};

class InputManager {
public:
    void begin();
    void update();

    // Consultas
    ButtonEvent event(ButtonId id) const   { return events[id]; }
    bool        held(ButtonId id) const    { return stableState[id]; }
    uint32_t    heldMs(ButtonId id) const;

    // Conveniencias
    bool pressed(ButtonId id) const     { return events[id] == EVT_PRESSED; }
    bool released(ButtonId id) const    { return events[id] == EVT_RELEASED; }
    bool repeating(ButtonId id) const   {
        return events[id] == EVT_PRESSED || events[id] == EVT_REPEAT;
    }
    bool longPressed(ButtonId id) const { return events[id] == EVT_LONG_PRESS; }

    void consume(ButtonId id);
    void resetAll();
    void injectVirtualState(ButtonId id, bool pressed);

    // Global termination logic
    bool cancelRequested();
    bool isCancelFlagPending() const { return cancelFlag; }
    int hijackedDigitalRead(uint8_t pin);

private:
    static const uint8_t PINS[BTN_COUNT];

    bool        stableState[BTN_COUNT]  = {false};
    bool        virtualState[BTN_COUNT] = {false};
    bool        rawPrev[BTN_COUNT]      = {false};
    uint32_t    lastEdgeMs[BTN_COUNT]   = {0};
    uint32_t    pressStartMs[BTN_COUNT] = {0};
    uint32_t    lastRepeatMs[BTN_COUNT] = {0};
    bool        longFired[BTN_COUNT]    = {false};
    ButtonEvent events[BTN_COUNT]       = {EVT_NONE};

    // Modifier logic
    bool backIsModifier = false;
    bool comboUsed = false;
    bool cancelFlag = false;
};

extern InputManager Input;

// Global digitalRead override to catch blocking loops safely
#define digitalRead(pin) Input.hijackedDigitalRead(pin)

#endif // INPUT_MANAGER_H