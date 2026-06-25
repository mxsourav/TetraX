#ifndef IR_MANAGER_H
#define IR_MANAGER_H

#include <Arduino.h>
#include "app_config.h"

struct AppIRData {
    int protocol;
    int address;
    int command;
    int flags;
};

class IRManager {
private:
    bool isReceiving;
    uint32_t jamPackets = 0;
    uint64_t jamActiveTimeMicros = 0;
    uint64_t jamTotalTimeMicros = 0;
    uint32_t sessionStartTimeMicros = 0;

    IRManager();
    void disableInterferingRadios();
    void enableInterferingRadios();

public:
    IRManager(const IRManager&) = delete;
    IRManager& operator=(const IRManager&) = delete;

    static IRManager& getInstance();

    void begin();
    
    void beginSession();
    void endSession();
    static bool sessionActive;
    
    // Puts the module into listen mode and quiets RF
    void startReceive();
    
    // Restores RF
    void stopReceive();

    // Polls for new data, returns true if signal acquired
    bool checkReceivedSignal(AppIRData* data, uint16_t* rawBuf, uint8_t* rawLen);

    // Transmit a raw array
    void sendRaw(const uint16_t* buffer, size_t length);

    // --- IR TOOLS UNIFIED INTERFACE ---
    
    // TV Remote Interface
    uint8_t getTvBrandCount();
    const char* getTvBrandName(uint8_t index);
    // cmdIndex: 0=PWR, 1=VOL+, 2=VOL-, 3=CH+, 4=CH-, 5=MUTE, 6=HOME, 7=BACK
    bool transmitTvCommand(uint8_t brandIndex, uint8_t cmdIndex);
    
    uint8_t getAcBrandCount();
    const char* getAcBrandName(uint8_t index);
    bool transmitAcCommand(uint8_t brandIndex, uint8_t cmdIndex, bool bypassCooldown = false);

    // True Carrier Flood
    void beginTvJammer();
    void jammerTickTv();
    
    // AC Protocol Flood
    void jammerTickAc();

    // Emergency Shutdown
    void emergencyStop();

    uint32_t getJamPacketsSent() const { return jamPackets; }
    uint8_t getJamDutyCycle() const;
    void resetJamStats() { 
        jamPackets = 0; 
        jamActiveTimeMicros = 0;
        jamTotalTimeMicros = 0;
        sessionStartTimeMicros = micros();
    }

    void replaySignal(const AppIRData* data);

};

#endif // IR_MANAGER_H
