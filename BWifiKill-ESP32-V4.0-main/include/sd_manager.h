#ifndef SD_MANAGER_H
#define SD_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "app_config.h"

class SDManager {
private:
    bool isMounted;

    // Private constructor for singleton
    SDManager();

    // SPI Coexistence
    void acquireSPI();
    void releaseSPI();

public:
    // Delete copy constructors
    SDManager(const SDManager&) = delete;
    SDManager& operator=(const SDManager&) = delete;

    static SDManager& getInstance();

    // Initializes the HSPI bus and mounts the SD card
    bool begin();

    // Isolated hardware test to verify functionality
    bool runIsolatedTest();

    // IR Persistence Methods
    bool saveIRDataBinary(const char* filename, const uint16_t* rawData, size_t length);
    bool loadIRDataBinary(const char* filename, uint16_t* buffer, size_t* outLength, size_t maxLen);

    bool isReady() const { return isMounted; }
};

#endif // SD_MANAGER_H
