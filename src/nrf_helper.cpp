#include "nrf_helper.h"
#include <SPI.h>

// Global pointer initialization
RF24* activeRadio = &jam1;

void safeNrfRecovery(RF24* radio, uint8_t restartChannel, bool isHopping) {
    if (!radio->isChipConnected()) {
        Serial.println("[NRF_HELPER] NRF offline or frozen. Reinitializing SPI and Radio...");
        
        // Re-init SPI Bus (using standard VSPI)
        SPI.begin();
        
        // Re-init Radio
        if (radio->begin()) {
            radio->setAutoAck(false);
            radio->setPALevel(RF24_PA_MAX, true);
            radio->setDataRate(RF24_1MBPS); // Default to 1MBPS, modules should reconfigure if needed
            radio->setCRCLength(RF24_CRC_DISABLED);
            
            if (isHopping && restartChannel > 0) {
                radio->startConstCarrier(RF24_PA_MAX, restartChannel);
            }
            Serial.println("[NRF_HELPER] Recovery successful.");
        } else {
            Serial.println("[NRF_HELPER] Recovery FAILED.");
        }
    }
}
