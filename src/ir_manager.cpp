#include "ir_manager.h"

// Enable sending specific protocols before including IRremote
#define SEND_SAMSUNG
#define SEND_SONY
#define SEND_LG
#define SEND_NEC

#include <IRremote.hpp>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_bt.h>

enum IRProtocolType {
    IR_PROTO_SAMSUNG,
    IR_PROTO_SONY,
    IR_PROTO_LG,
    IR_PROTO_NEC_ANDROID,
    IR_PROTO_NEC_GENERIC,
    IR_PROTO_AC_DEMO
};

struct TVBrand {
    const char* name;
    IRProtocolType protocol;
    uint16_t address;
    uint16_t commands[8];
    uint8_t repeats;
};

static const TVBrand TV_BRANDS[] = {
    { "Samsung", IR_PROTO_SAMSUNG,     0x0707, {0x02, 0x07, 0x0B, 0x12, 0x10, 0x0F, 0x79, 0x58}, 1 },
    { "Sony",    IR_PROTO_SONY,        0x01,   {0x15, 0x12, 0x13, 0x10, 0x11, 0x14, 0x60, 0x23}, 2 },
    { "LG",      IR_PROTO_LG,          0x04,   {0x08, 0x02, 0x03, 0x00, 0x01, 0x09, 0x0A, 0x14}, 1 },
    { "Android", IR_PROTO_NEC_ANDROID, 0x00,   {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}, 1 },
    { "Generic", IR_PROTO_NEC_GENERIC, 0x00,   {0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21}, 1 }
};
static const uint8_t NUM_TV_BRANDS = sizeof(TV_BRANDS) / sizeof(TV_BRANDS[0]);

struct ACBrand {
    const char* name;
    IRProtocolType protocol;
    uint16_t address;
    uint16_t commands[6];
    uint8_t repeats;
};

static const ACBrand AC_BRANDS[] = {
    { "Voltas",      IR_PROTO_AC_DEMO, 0x01, {0x10, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "Blue Star",   IR_PROTO_AC_DEMO, 0x02, {0x20, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "LG",          IR_PROTO_AC_DEMO, 0x03, {0x30, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "Samsung",     IR_PROTO_AC_DEMO, 0x04, {0x40, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "Daikin",      IR_PROTO_AC_DEMO, 0x05, {0x50, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "Panasonic",   IR_PROTO_AC_DEMO, 0x06, {0x60, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "Lloyd",       IR_PROTO_AC_DEMO, 0x07, {0x70, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "Hitachi",     IR_PROTO_AC_DEMO, 0x08, {0x80, 0x00, 0x00, 0x00, 0x00, 0x00}, 1 },
    { "Generic HVAC",IR_PROTO_AC_DEMO, 0x88, {0x12, 0x13, 0x14, 0x15, 0x16, 0x17}, 1 }
};
static const uint8_t NUM_AC_BRANDS = sizeof(AC_BRANDS) / sizeof(AC_BRANDS[0]);
static uint32_t lastTxTime = 0;
#define TX_COOLDOWN_MS 400

IRManager::IRManager() : isReceiving(false) {}

IRManager& IRManager::getInstance() {
    static IRManager instance;
    return instance;
}

void IRManager::begin() {
    Serial.println("[IRManager] Initializing IR Transceiver...");
    IrReceiver.begin(AppConfig::IR_RX_PIN, ENABLE_LED_FEEDBACK);
    
    // STRICT ISOLATION: The receiver ISR must remain totally dead globally
    // unless explicitly started by the IR RECEIVER app.
    IrReceiver.stop(); 
    
    IrSender.begin(AppConfig::IR_TX_PIN);
    Serial.println("[IRManager] Initialized.");
}

bool IRManager::sessionActive = false;

void IRManager::beginSession() {
    if (sessionActive) return;
    Serial.println("[IR SESSION START] Disabling RF Systems");
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_OFF);
    esp_bt_controller_disable();
    sessionActive = true;
}

void IRManager::endSession() {
    if (!sessionActive) return;
    Serial.println("[IR SESSION END] Restoring RF Systems");
    // Restore logic (WiFi on-demand handled elsewhere)
    sessionActive = false;
}

void IRManager::disableInterferingRadios() {
    // Deprecated per burst, handled by session
}

void IRManager::enableInterferingRadios() {
    // Deprecated per burst, handled by session
}

void IRManager::startReceive() {
    if (isReceiving) return;
    IrReceiver.start();
    isReceiving = true;
    Serial.println("[IRManager] Started receiving IR...");
}

void IRManager::stopReceive() {
    if (!isReceiving) return;
    IrReceiver.stop();
    isReceiving = false;
    Serial.println("[IRManager] Stopped receiving IR.");
}

bool IRManager::checkReceivedSignal(AppIRData* appData, uint16_t* rawBuf, uint8_t* rawLen) {
    if (IrReceiver.decode()) {
        if (appData) {
            appData->protocol = IrReceiver.decodedIRData.protocol;
            appData->address = IrReceiver.decodedIRData.address;
            appData->command = IrReceiver.decodedIRData.command;
            appData->flags = IrReceiver.decodedIRData.flags;
        }
        IrReceiver.resume(); // Prepare for next signal
        return true;
    }
    return false;
}

void IRManager::sendRaw(const uint16_t* buffer, size_t length) {
    if (isReceiving) stopReceive();
    Serial.printf("[IRManager] Sending RAW IR signal (%d ticks)...\n", length);
    IrSender.sendRaw(buffer, length, 38);
}

uint8_t IRManager::getTvBrandCount() { return NUM_TV_BRANDS; }
const char* IRManager::getTvBrandName(uint8_t index) {
    if (index >= NUM_TV_BRANDS) return "Unknown";
    return TV_BRANDS[index].name;
}

bool IRManager::transmitTvCommand(uint8_t brandIndex, uint8_t cmdIndex) {
    if (brandIndex >= NUM_TV_BRANDS || cmdIndex > 7) return false;
    if (millis() - lastTxTime < TX_COOLDOWN_MS) return false;

    bool wasReceiving = isReceiving;
    if (wasReceiving) stopReceive();

    const TVBrand& brand = TV_BRANDS[brandIndex];
    uint16_t cmd = brand.commands[cmdIndex];

    switch(brand.protocol) {
        case IR_PROTO_SAMSUNG: IrSender.sendSamsung(brand.address, cmd, brand.repeats); break;
        case IR_PROTO_SONY:    IrSender.sendSony(brand.address, cmd, brand.repeats); break;
        case IR_PROTO_LG:      IrSender.sendLG(brand.address, cmd, brand.repeats); break;
        case IR_PROTO_NEC_ANDROID:
        case IR_PROTO_NEC_GENERIC:
            IrSender.sendNEC(brand.address, cmd, brand.repeats); break;
        default: break;
    }

    if (wasReceiving) startReceive();
    lastTxTime = millis();
    return true;
}

uint8_t IRManager::getAcBrandCount() { return NUM_AC_BRANDS; }
const char* IRManager::getAcBrandName(uint8_t index) {
    if (index >= NUM_AC_BRANDS) return "Unknown";
    return AC_BRANDS[index].name;
}

bool IRManager::transmitAcCommand(uint8_t brandIndex, uint8_t cmdIndex, bool bypassCooldown) {
    if (brandIndex >= NUM_AC_BRANDS || cmdIndex > 5) return false;
    
    if (!bypassCooldown && millis() - lastTxTime < TX_COOLDOWN_MS) return false;

    bool wasReceiving = isReceiving;
    if (wasReceiving) stopReceive();

    const ACBrand& brand = AC_BRANDS[brandIndex];
    uint16_t cmd = brand.commands[cmdIndex];

    if (brand.protocol == IR_PROTO_AC_DEMO) {
        IrSender.sendNEC(brand.address, cmd, brand.repeats);
    }

    if (wasReceiving) startReceive();
    lastTxTime = millis();
    return true;
}

uint8_t IRManager::getJamDutyCycle() const {
    if (jamTotalTimeMicros == 0) return 0;
    return (uint8_t)((jamActiveTimeMicros * 100) / jamTotalTimeMicros);
}

void IRManager::beginTvJammer() {
    // Initialize the hardware PWM timer ONCE safely
    IrSender.enableIROut(38);
}

void IRManager::jammerTickTv() {
    uint32_t tStart = micros();
    
    // SAFE HIGH-DENSITY MICRO-BURST JAMMING (RAND-OOK)
    // Limits: Max mark <= 5ms
    uint16_t burstMicros = random(2000, 5001); // 2 to 5 ms
    uint16_t gapMicros = random(300, 1001);   // 0.3 to 1 ms
    
    IrSender.mark(burstMicros);
    IrSender.space(gapMicros);
    
    uint32_t tEnd = micros();
    
    // Calculate duty cycle roughly
    jamActiveTimeMicros += burstMicros;
    jamTotalTimeMicros += (burstMicros + gapMicros);
    jamPackets++;
}

void IRManager::jammerTickAc() {
    uint32_t tStart = micros();
    
    // India-prioritized AC flood rotation
    // 0=Generic (fallback), but we rotate through configured ones
    static uint8_t currentAcBrand = 0;
    
    // Bypass cooldown for protocol flood
    transmitAcCommand(currentAcBrand, 0, true);
    
    currentAcBrand++;
    if (currentAcBrand >= NUM_AC_BRANDS) {
        currentAcBrand = 0;
    }
    
    uint32_t tEnd = micros();
    // Estimate AC packet duty cycle heavily because library blocking delays
    jamActiveTimeMicros += ((tEnd - tStart) * 70) / 100; // Roughly 70% active IR duty during protocol send
    jamTotalTimeMicros += (tEnd - tStart);
    jamPackets++;
}

void IRManager::emergencyStop() {
    IrSender.space(0);
}

void IRManager::replaySignal(const AppIRData* data) {
    if (!data) return;

    bool wasReceiving = isReceiving;
    if (wasReceiving) stopReceive();

    if (data->protocol == decode_type_t::NEC) {
         IrSender.sendNEC(data->address, data->command, 0);
    } else if (data->protocol == decode_type_t::SAMSUNG) {
         IrSender.sendSamsung(data->address, data->command, 0);
    } else if (data->protocol == decode_type_t::SONY) {
         IrSender.sendSony(data->address, data->command, 2);
    } else if (data->protocol == decode_type_t::LG) {
         IrSender.sendLG(data->address, data->command, 0);
    }

    if (wasReceiving) startReceive();
}
