#ifndef SLAVE_MANAGER_H
#define SLAVE_MANAGER_H

#include <Arduino.h>

// Protocol commands (Abstracted RF tasks)
enum SlaveCommand : uint8_t {
    CMD_HELO_MASTER = 0x01,
    CMD_HELO_SLAVE  = 0x02,
    CMD_HEARTBEAT   = 0x03,
    CMD_HEARTBEAT_ACK = 0x04,
    
    CMD_RF_SCAN     = 0x10,
    CMD_RF_TX       = 0x11,
    CMD_RF_ANALYZE  = 0x12,
    CMD_RF_PING     = 0x13,
    CMD_RF_STREAM   = 0x14,
    
    CMD_STOP_ALL    = 0x30,
    CMD_TELEMETRY   = 0x40
};

// Protocol constants
constexpr uint8_t PROTO_V1 = 0x01;
constexpr uint32_t SLAVE_TIMEOUT_MS = 3500;
constexpr uint32_t HEARTBEAT_INTERVAL_MS = 1000;
constexpr uint32_t BOOT_HANDSHAKE_MS = 1000;

// Maximum payload size for a queued job
constexpr uint8_t MAX_JOB_PAYLOAD = 32;

struct RfJob {
    uint8_t cmd;
    uint8_t payload[MAX_JOB_PAYLOAD];
    uint8_t len;
    bool active;
};

// Initializes UART and begins listening for handshake
void slaveManagerInit();

// Checks if we should transition to slave mode during boot
bool slaveManagerWaitForMaster();

// Must be called in the main loop to handle slave mode
void slaveManagerLoop();

// Returns true if the device is currently in slave mode
bool isSlaveModeActive();

// Send telemetry or responses back to master with SLIP framing
void slaveManagerSendPacket(uint8_t command, const uint8_t* payload, uint16_t length);

#endif
