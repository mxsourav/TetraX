#ifndef NRF_HELPER_H
#define NRF_HELPER_H

#include <Arduino.h>
#include <RF24.h>

// Global radio instance
extern RF24 jam1;

// Global pointer for modularity
extern RF24* activeRadio;

// Safely attempts to recover the radio if it stops responding
void safeNrfRecovery(RF24* radio, uint8_t restartChannel = 0, bool isHopping = true);

#endif // NRF_HELPER_H
