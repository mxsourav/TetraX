#include "slave_manager.h"
#include "app_config.h"
#include <U8g2lib.h>
#include "ui_theme.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern void oledMirrorSync();

static bool slaveModeActive = false;
static uint32_t lastHeartbeatMs = 0;
static uint32_t lastPingSentMs = 0;
static uint32_t packetCount = 0;
static uint32_t errorCount = 0;
static uint8_t  txSequence = 0;
static uint8_t  lastRxSequence = 0;

static uint32_t lastHeloMs = 0;
static uint8_t  heloCount = 0;

static RfJob activeJob = {0, {0}, 0, false};

// SLIP framing constants
constexpr uint8_t SLIP_END     = 0xC0;
constexpr uint8_t SLIP_ESC     = 0xDB;
constexpr uint8_t SLIP_ESC_END = 0xDC;
constexpr uint8_t SLIP_ESC_ESC = 0xDD;

// Parser state
static uint8_t rxBuffer[256];
static uint16_t rxIndex = 0;
static bool escapeNext = false;

void slaveManagerInit() {
    // 115200 is default, but we can bump this later
    Serial2.begin(115200, SERIAL_8N1, AppConfig::SLAVE_RX, AppConfig::SLAVE_TX);
    slaveModeActive = false;
    rxIndex = 0;
    escapeNext = false;
}

static void sendSlipByte(uint8_t b) {
    if (b == SLIP_END) {
        Serial2.write(SLIP_ESC);
        Serial2.write(SLIP_ESC_END);
    } else if (b == SLIP_ESC) {
        Serial2.write(SLIP_ESC);
        Serial2.write(SLIP_ESC_ESC);
    } else {
        Serial2.write(b);
    }
}

void slaveManagerSendPacket(uint8_t command, const uint8_t* payload, uint16_t length) {
    Serial2.write(SLIP_END); // Start delimiter

    txSequence++;
    uint8_t csum = 0;

    // SEQ
    sendSlipByte(txSequence);
    csum ^= txSequence;

    // CMD
    sendSlipByte(command);
    csum ^= command;

    // LEN (capped at 250 to fit in 1 byte for simplicity, or 2 bytes if needed. Using 1 byte here.)
    uint8_t len8 = (uint8_t)(length & 0xFF);
    sendSlipByte(len8);
    csum ^= len8;

    // PAYLOAD
    for (uint16_t i = 0; i < length; i++) {
        sendSlipByte(payload[i]);
        csum ^= payload[i];
    }

    // CHECKSUM
    sendSlipByte(csum);

    Serial2.write(SLIP_END); // End delimiter
}

static void resetParser() {
    rxIndex = 0;
    escapeNext = false;
}

static void processCommand(uint8_t seq, uint8_t cmd, const uint8_t* payload, uint8_t len) {
    lastHeartbeatMs = millis();
    lastRxSequence = seq;
    packetCount++;

    if (cmd == CMD_HELO_MASTER) {
        if (!slaveModeActive) {
            // Check protocol version
            if (len == 0 || payload[0] != PROTO_V1) {
                return; // Mismatched protocol or invalid payload
            }
            
            // Debounce: require 2 packets within 500ms
            if (millis() - lastHeloMs > 500) {
                heloCount = 0;
            }
            heloCount++;
            lastHeloMs = millis();
            
            if (heloCount >= 2) {
                slaveModeActive = true;
                // Acknowledge with our protocol version
                uint8_t responsePayload[] = { PROTO_V1 };
                slaveManagerSendPacket(CMD_HELO_SLAVE, responsePayload, sizeof(responsePayload));
            }
        }
    }
    else if (cmd == CMD_HEARTBEAT) {
        // Master sent heartbeat, acknowledge it
        slaveManagerSendPacket(CMD_HEARTBEAT_ACK, nullptr, 0);
    }
    else if (cmd == CMD_HEARTBEAT_ACK) {
        // We received ACK from master
    }
    else {
        // Queue RF worker commands instead of executing directly
        switch (cmd) {
            case CMD_RF_SCAN:
            case CMD_RF_TX:
            case CMD_RF_ANALYZE:
            case CMD_STOP_ALL:
                activeJob.cmd = cmd;
                activeJob.len = min((uint8_t)MAX_JOB_PAYLOAD, len);
                if (activeJob.len > 0 && payload != nullptr) {
                    memcpy(activeJob.payload, payload, activeJob.len);
                }
                activeJob.active = true;
                break;
        }
    }
}

static void parseDecodedFrame(const uint8_t* buf, uint16_t len) {
    // Minimum frame: SEQ(1) + CMD(1) + LEN(1) + CSUM(1) = 4 bytes
    if (len < 4) {
        errorCount++;
        return; 
    }

    uint8_t seq = buf[0];
    uint8_t cmd = buf[1];
    uint8_t payloadLen = buf[2];
    uint8_t csumRx = buf[len - 1];

    if (len != (uint16_t)(payloadLen + 4)) {
        errorCount++; // Length mismatch
        return;
    }

    uint8_t csumCalc = 0;
    for (uint16_t i = 0; i < len - 1; i++) {
        csumCalc ^= buf[i];
    }

    if (csumCalc != csumRx) {
        errorCount++; // Checksum failed
        return;
    }

    processCommand(seq, cmd, &buf[3], payloadLen);
}

static void pollSerial() {
    while (Serial2.available() > 0) {
        uint8_t b = Serial2.read();

        if (b == SLIP_END) {
            if (rxIndex > 0) {
                parseDecodedFrame(rxBuffer, rxIndex);
            }
            resetParser();
        } 
        else if (b == SLIP_ESC) {
            escapeNext = true;
        } 
        else {
            if (escapeNext) {
                if (b == SLIP_ESC_END) b = SLIP_END;
                else if (b == SLIP_ESC_ESC) b = SLIP_ESC;
                escapeNext = false;
            }

            if (rxIndex < sizeof(rxBuffer)) {
                rxBuffer[rxIndex++] = b;
            } else {
                // Buffer overflow, drop packet and recover gracefully
                errorCount++;
                resetParser();
            }
        }
    }
}

bool slaveManagerWaitForMaster() {
    slaveManagerInit();
    
    // Listen for the handshake for a short window during boot
    uint32_t startMs = millis();
    while (millis() - startMs < BOOT_HANDSHAKE_MS) {
        pollSerial();
        if (slaveModeActive) {
            return true;
        }
        delay(10);
    }
    return false;
}

bool isSlaveModeActive() {
    return slaveModeActive;
}

void slaveManagerLoop() {
    if (!slaveModeActive) {
        // Hot-plugging check
        pollSerial();
        return;
    }

    // In Slave Mode
    pollSerial();

    uint32_t now = millis();

    // Send heartbeat to master periodically
    if (now - lastPingSentMs > HEARTBEAT_INTERVAL_MS) {
        lastPingSentMs = now;
        slaveManagerSendPacket(CMD_HEARTBEAT, nullptr, 0);
    }

    // Check Heartbeat Timeout
    if (now - lastHeartbeatMs > SLAVE_TIMEOUT_MS) {
        // Connection lost, reboot to clean state
        Serial.println("[SLAVE] Heartbeat lost. Rebooting to standalone.");
        ESP.restart();
    }

    // Draw minimal dashboard to reduce CPU load (priority goes to RF tasks)
    // Reduce refresh rate heavily (every 500ms) to prioritize UART and RF
    static uint32_t lastDraw = 0;
    if (now - lastDraw > 500) {
        lastDraw = now;
        u8g2.clearBuffer();
        
        UiTheme::drawHeader(u8g2, "tetrax", "SLAVE");
        
        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(10, 25, "LINKED TO BLUEBOX");
        
        char buf[32];
        snprintf(buf, sizeof(buf), "PKTS: %lu", packetCount);
        u8g2.drawStr(10, 40, buf);

        snprintf(buf, sizeof(buf), "ERR : %lu", errorCount);
        u8g2.drawStr(10, 52, buf);
        
        // Activity indicator
        if ((now - lastHeartbeatMs) < 1000) {
            u8g2.drawBox(110, 20, 8, 8);
        } else {
            u8g2.drawFrame(110, 20, 8, 8);
        }

        u8g2.sendBuffer();
        oledMirrorSync();
    }
    
    // Execute RF Job Queue asynchronously
    if (activeJob.active) {
        activeJob.active = false; // Mark as handled
        
        switch (activeJob.cmd) {
            case CMD_RF_SCAN:
                // TODO: hook into wifiscan.cpp or nrf_helper.cpp
                Serial.println("[SLAVE_WORKER] Executing RF_SCAN");
                break;
                
            case CMD_RF_TX:
                // TODO: hook into jammer.cpp or total_jammer.cpp
                Serial.println("[SLAVE_WORKER] Executing RF_TX");
                break;
                
            case CMD_RF_ANALYZE:
                // TODO: hook into spectrograph.cpp
                Serial.println("[SLAVE_WORKER] Executing RF_ANALYZE");
                break;
                
            case CMD_STOP_ALL:
                // TODO: stop activeRadio operations safely
                Serial.println("[SLAVE_WORKER] Executing STOP_ALL");
                break;
        }
    }
}
