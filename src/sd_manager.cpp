#include "sd_manager.h"

SDManager::SDManager() : isMounted(false) {}

SDManager& SDManager::getInstance() {
    static SDManager instance;
    return instance;
}

void SDManager::acquireSPI() {
    // Disable NRF modules first
    pinMode(AppConfig::NRF1_CSN, OUTPUT);
    digitalWrite(AppConfig::NRF1_CSN, HIGH);
    pinMode(AppConfig::NRF2_CSN, OUTPUT);
    digitalWrite(AppConfig::NRF2_CSN, HIGH);
    
    // SPI belongs to SD now
    Serial.println("[SPI_OWNER] SD");
}

void SDManager::releaseSPI() {
    // Deselect SD
    digitalWrite(AppConfig::SD_CS, HIGH);
    Serial.println("[SPI_OWNER] RELEASED (SD)");
}

bool SDManager::begin() {
    if (isMounted) return true;

    Serial.println("[SDManager] Initializing SD Card...");

    acquireSPI();
    Serial.println("[SD_INIT]");

    pinMode(AppConfig::SD_CS, OUTPUT);
    digitalWrite(AppConfig::SD_CS, HIGH); // De-select initially

    // Attempt to mount the SD card using global SPI at 1MHz for breadboard stability
    if (!SD.begin(AppConfig::SD_CS, SPI, 1000000)) {
        Serial.println("[SDManager] ERROR: SD Card mount failed!");
        releaseSPI();
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[SDManager] ERROR: No SD card attached.");
        releaseSPI();
        return false;
    }

    Serial.printf("[SDManager] SD Card initialized. Size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    isMounted = true;
    releaseSPI();
    return true;
}

bool SDManager::runIsolatedTest() {
    if (!isMounted) {
        Serial.println("[SDManager] Test aborted. Card not mounted.");
        return false;
    }

    Serial.println("[SDManager] Running isolated hardware test...");
    const char* testFile = "/test_sd.txt";

    acquireSPI();
    Serial.println("[SD_WRITE] test_sd.txt");

    // Write Test
    File file = SD.open(testFile, FILE_WRITE);
    if (!file) {
        Serial.println("[SDManager] ERROR: Failed to open file for writing.");
        releaseSPI();
        return false;
    }
    if (!file.println("BWifiKill V4 SD Test OK")) {
        Serial.println("[SDManager] ERROR: Write failed.");
        file.close();
        releaseSPI();
        return false;
    }
    file.close();
    releaseSPI();
    Serial.println("[SDManager] Write test passed.");

    acquireSPI();
    Serial.println("[SD_READ] test_sd.txt");
    // Read Test
    file = SD.open(testFile);
    if (!file) {
        Serial.println("[SDManager] ERROR: Failed to open file for reading.");
        releaseSPI();
        return false;
    }
    String content = file.readStringUntil('\n');
    file.close();

    if (content.indexOf("BWifiKill V4 SD Test OK") >= 0) {
        Serial.println("[SDManager] Read test passed.");
        
        // Clean up
        SD.remove(testFile);
        releaseSPI();
        return true;
    } else {
        Serial.println("[SDManager] ERROR: Read verification failed.");
        releaseSPI();
        return false;
    }
}

bool SDManager::saveIRDataBinary(const char* filename, const uint16_t* rawData, size_t length) {
    if (!isMounted) return false;

    acquireSPI();
    Serial.printf("[SD_WRITE] %s\n", filename);

    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        releaseSPI();
        return false;
    }

    size_t written = file.write((const uint8_t*)rawData, length * sizeof(uint16_t));
    file.close();
    releaseSPI();

    return written == (length * sizeof(uint16_t));
}

bool SDManager::loadIRDataBinary(const char* filename, uint16_t* buffer, size_t* outLength, size_t maxLen) {
    if (!isMounted) return false;

    acquireSPI();
    Serial.printf("[SD_READ] %s\n", filename);

    File file = SD.open(filename, FILE_READ);
    if (!file) {
        releaseSPI();
        return false;
    }

    size_t fileSize = file.size();
    size_t items = fileSize / sizeof(uint16_t);

    if (items > maxLen) {
        items = maxLen; // Prevent buffer overflow
    }

    size_t readBytes = file.read((uint8_t*)buffer, items * sizeof(uint16_t));
    file.close();
    releaseSPI();

    if (outLength) {
        *outLength = items;
    }

    return readBytes == (items * sizeof(uint16_t));
}
