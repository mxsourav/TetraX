#include "raone_link.h"
#include "app_config.h"
#include <U8g2lib.h>
#include "oled_mirror.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

static bool _serial2Started = false;

void RaoneLink::begin() {
    if (!_serial2Started) {
        Serial2.begin(115200, SERIAL_8N1, AppConfig::SLAVE_RX, AppConfig::SLAVE_TX);
        _serial2Started = true;
        Serial.printf("[RAONE LINK] Serial2 initialized on RX=%d, TX=%d @ 115200\n", AppConfig::SLAVE_RX, AppConfig::SLAVE_TX);
    }
    _connected = false;
    _rxBuffer = "";
}

bool RaoneLink::performBootHandshake() {
    begin(); // Ensure Serial2 is running

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(5, 25, "Searching for");
    u8g2.drawStr(5, 40, "RAONE 5G...");
    u8g2.sendBuffer();
    oledMirrorSync();

    // Flush old rx buffer
    while (Serial2.available() > 0) Serial2.read();

    unsigned long startMs = millis();
    unsigned long lastPingMs = 0;
    bool found = false;

    Serial.println("[RAONE LINK] Starting handshake search...");

    while (millis() - startMs < 2500) {
        // Send ping every 100ms immediately starting from t=0
        if (millis() - lastPingMs >= 100) {
            Serial2.print("PING_RAONE\n");
            Serial2.flush();
            lastPingMs = millis();
        }

        // Check for ACK
        while (Serial2.available() > 0) {
            char c = (char)Serial2.read();
            if (c == '\n' || c == '\r') {
                if (_rxBuffer.length() > 0) {
                    _rxBuffer.trim();
                    Serial.printf("[RAONE LINK RX]: \"%s\"\n", _rxBuffer.c_str());
                    if (_rxBuffer.indexOf("RAONE") >= 0 || _rxBuffer.indexOf("HEARTBEAT") >= 0 || _rxBuffer.indexOf("Up:") >= 0 || _rxBuffer.indexOf("READY") >= 0 || _rxBuffer.indexOf("STATUS:") >= 0) {
                        found = true;
                        break;
                    }
                    _rxBuffer = "";
                }
            } else if (_rxBuffer.length() < 120) {
                _rxBuffer += c;
                // Check substring on-the-fly
                if (_rxBuffer.indexOf("RAONE") >= 0 || _rxBuffer.indexOf("HEARTBEAT") >= 0 || _rxBuffer.indexOf("Up:") >= 0 || _rxBuffer.indexOf("READY") >= 0 || _rxBuffer.indexOf("STATUS:") >= 0) {
                    found = true;
                    break;
                }
            }
        }
        
        if (found) break;
        delay(10);
    }

    _connected = found;
    _rxBuffer = ""; // Clear buffer
    if (_connected) {
        Serial.println("[RAONE LINK] *** HANDSHAKE SUCCESS! RAONE CONNECTED ***");
    } else {
        Serial.println("[RAONE LINK] Handshake timed out (no response)");
    }
    return _connected;
}

bool RaoneLink::isConnected() {
    return _connected;
}

void RaoneLink::sendCommand(const String& cmd) {
    if (_connected) {
        Serial2.print(cmd + "\n");
    }
}

void RaoneLink::update() {
    // Always process Serial2 — enables hot-plug detection
    while (Serial2.available() > 0) {
        char c = Serial2.read();
        if (c == '\n') {
            _rxBuffer.trim();

            // Hot-connect: if we see RAONE response, heartbeat, or status, mark connected
            if (!_connected && (_rxBuffer.indexOf("RAONE") >= 0 || _rxBuffer.indexOf("HEARTBEAT") >= 0 || _rxBuffer.indexOf("Up:") >= 0 || _rxBuffer.indexOf("READY") >= 0 || _rxBuffer.indexOf("STATUS:") >= 0)) {
                _connected = true;
                Serial.println("[RAONE LINK] *** HOT-CONNECT! RAONE linked successfully! ***");
            }

            if (_rxBuffer.startsWith("STATUS:LIVE:")) {
                // Parse: STATUS:LIVE:<ch>,<sent>,<fail>,<pps>
                int firstComma  = _rxBuffer.indexOf(',', 12);
                int secondComma = _rxBuffer.indexOf(',', firstComma + 1);
                int thirdComma  = _rxBuffer.indexOf(',', secondComma + 1);

                if (firstComma > 0 && secondComma > 0 && thirdComma > 0) {
                    _raoneChannel = _rxBuffer.substring(12, firstComma).toInt();
                    _liveTxCount  = _rxBuffer.substring(firstComma + 1, secondComma).toInt();
                    _failCount    = _rxBuffer.substring(secondComma + 1, thirdComma).toInt();
                    _livePps      = _rxBuffer.substring(thirdComma + 1).toInt();
                    _isBusy = true;
                }
            } else if (_rxBuffer.startsWith("STATUS:TX:")) {
                _liveTxCount = _rxBuffer.substring(10).toInt();
            } else if (_rxBuffer.startsWith("STATUS:CH:")) {
                _raoneChannel = _rxBuffer.substring(10).toInt();
            } else if (_rxBuffer.startsWith("STATUS:PPS:")) {
                _livePps = _rxBuffer.substring(11).toInt();
            } else if (_rxBuffer.startsWith("STATUS:STOPPED") || _rxBuffer == "STATUS:IDLE") {
                _isBusy = false;
                _livePps = 0;
            }
            Serial.println("[RAONE] " + _rxBuffer);
            _rxBuffer = "";
        } else {
            _rxBuffer += c;
        }
    }
}



