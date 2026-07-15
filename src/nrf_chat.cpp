#include "nrf_chat.h"
#include "oled_mirror.h"
#include "input_manager.h"
#include "ui_theme.h"
#include "nrf_helper.h"
#include <U8g2lib.h>
#include <WiFi.h>
#include <string.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

static const uint16_t CHAT_ACK_WAIT_MS = 65;
static const uint16_t CHAT_DRAW_INTERVAL_MS = 120;
static const uint8_t CHAT_CHANNELS[] = {2, 26, 40, 60, 76};

static const uint8_t ADDR_MASTER[6] = {'B', 'W', 'F', 'M', '1', 0};
static const uint8_t ADDR_SLAVE[6]  = {'B', 'W', 'F', 'S', '1', 0};

static const char* CHAT_MESSAGES[] = {
    "HOLA",
    "OK",
    "LISTO",
    "RECIBIDO",
    "ACERCATE",
    "ALEJATE",
    "CANAL LIMPIO",
    "PRUEBA LINK",
    "DASHBOARD ON",
    "TERMINADO"
};

static const uint8_t CHAT_MESSAGE_COUNT = sizeof(CHAT_MESSAGES) / sizeof(CHAT_MESSAGES[0]);

enum ChatScreen : uint8_t {
    CHAT_SETUP = 0,
    CHAT_ACTIVE
};

struct __attribute__((packed)) ChatPacket {
    uint8_t  magic;
    uint8_t  type;
    uint16_t seq;
    uint8_t  fromRole;
    uint8_t  msgId;
    char     text[20];
};

static ChatScreen screen = CHAT_SETUP;
static uint8_t roleSelection = 0;
static uint8_t fieldSelection = 0;
static uint8_t channelSelection = 4;
static uint8_t selectedMessage = 0;
static bool radioReady = false;
static bool radioBeginOk = false;
static bool lastAckOk = false;
static uint16_t txSeq = 0;
static uint16_t lastRxSeq[2] = {0, 0};
static uint32_t txCount = 0;
static uint32_t ackCount = 0;
static uint32_t rxCount = 0;
static uint32_t lastRxMs = 0;
static uint32_t lastTxMs = 0;
static uint32_t lastDrawMs = 0;
static char lastInbox[21] = "";
static char lastOutbox[21] = "";

// RX window timing for half-duplex
static const uint16_t RX_WINDOW_MS = 50;
static uint32_t lastRxWindowMs = 0;

static uint8_t selectedChannel() {
    return CHAT_CHANNELS[channelSelection];
}

static const uint8_t* ownAddress() {
    return roleSelection == 0 ? ADDR_MASTER : ADDR_SLAVE;
}

static const uint8_t* peerAddress() {
    return roleSelection == 0 ? ADDR_SLAVE : ADDR_MASTER;
}

static void copyText(char* dst, size_t len, const char* src) {
    if (len == 0) return;
    strncpy(dst, src ? src : "", len - 1);
    dst[len - 1] = 0;
}

static void resetChatState() {
    selectedMessage = 0;
    lastAckOk = false;
    txSeq = 0;
    lastRxSeq[0] = 0;
    lastRxSeq[1] = 0;
    txCount = 0;
    ackCount = 0;
    rxCount = 0;
    lastRxMs = 0;
    lastTxMs = 0;
    lastDrawMs = 0;
    lastRxWindowMs = 0;
    lastInbox[0] = 0;
    lastOutbox[0] = 0;
}

static void configureRadio() {
    WiFi.mode(WIFI_OFF);
    activeRadio->stopConstCarrier();
    activeRadio->stopListening();

    radioBeginOk = activeRadio->begin();
    activeRadio->powerDown();
    delay(5);
    activeRadio->powerUp();
    activeRadio->setAddressWidth(5);
    activeRadio->setAutoAck(false);
    activeRadio->setRetries(0, 0);
    activeRadio->setPayloadSize(sizeof(ChatPacket));
    activeRadio->setChannel(selectedChannel());
    activeRadio->setDataRate(RF24_250KBPS);
    activeRadio->setPALevel(RF24_PA_HIGH);
    activeRadio->setCRCLength(RF24_CRC_16);
    activeRadio->openWritingPipe(peerAddress());
    activeRadio->openReadingPipe(1, ownAddress());
    activeRadio->flush_rx();
    activeRadio->flush_tx();
    activeRadio->startListening();
    radioReady = true;
}

static bool shouldDrawNow() {
    uint32_t now = millis();
    if (lastDrawMs == 0 || now - lastDrawMs >= CHAT_DRAW_INTERVAL_MS) {
        lastDrawMs = now;
        return true;
    }
    return false;
}

static void drawSetup() {
    UiTheme::drawHeader(u8g2, "NRF CHAT", "SET");

    u8g2.setFont(u8g2_font_5x7_tr);
    const char* role = roleSelection == 0 ? "MAESTRO" : "SLAVE";
    char channel[8];
    snprintf(channel, sizeof(channel), "CH%u", selectedChannel());

    const char* labels[] = {"ROL", "CANAL"};
    const char* values[] = {role, channel};

    for (uint8_t i = 0; i < 2; i++) {
        int y = 25 + i * 12;
        int x = 8;
        int w = 112;
        if (fieldSelection == i) {
            u8g2.drawBox(x, y - 8, w, 10);
            u8g2.setDrawColor(0);
        }
        u8g2.drawStr(x + 4, y, labels[i]);
        u8g2.drawStr(x + 51, y, values[i]);
        u8g2.setDrawColor(1);
    }

    u8g2.drawHLine(10, 59, 108);
    uint8_t markerX = 32 + fieldSelection * 52;
    u8g2.drawBox(markerX, 57, 8, 4);
}

static void drawMessageRow(uint8_t msgIndex, uint8_t row, bool selected) {
    int y = 22 + row * 8;
    if (selected) {
        u8g2.drawBox(0, y - 7, 128, 9);
        u8g2.setDrawColor(0);
    }
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(2, y, selected ? ">" : " ");
    u8g2.drawStr(10, y, CHAT_MESSAGES[msgIndex]);
    if (selected) u8g2.setDrawColor(1);
}

static void drawChat() {
    char status[12];
    snprintf(status, sizeof(status), radioBeginOk ? "%c" : "RF ERR",
             roleSelection == 0 ? 'M' : 'S');
    UiTheme::drawHeader(u8g2, "NRF CHAT", status);

    int first = selectedMessage - 2;
    if (first < 0) first = 0;
    if (first > (int)CHAT_MESSAGE_COUNT - 4) first = max(0, (int)CHAT_MESSAGE_COUNT - 4);

    for (uint8_t row = 0; row < 4; row++) {
        uint8_t idx = first + row;
        if (idx >= CHAT_MESSAGE_COUNT) break;
        drawMessageRow(idx, row, idx == selectedMessage);
    }

    u8g2.setFont(u8g2_font_4x6_tf);
    char rxLine[24];
    char txLine[24];
    if (lastInbox[0]) snprintf(rxLine, sizeof(rxLine), "RX:%s", lastInbox);
    else              snprintf(rxLine, sizeof(rxLine), "RX:-- CH%u", selectedChannel());

    if (lastOutbox[0]) {
        snprintf(txLine, sizeof(txLine), "TX:%s %s", lastOutbox, lastAckOk ? "OK" : "...");
    } else {
        snprintf(txLine, sizeof(txLine), "TX:%lu RX:%lu", txCount, rxCount);
    }

    u8g2.drawStr(2, 55, rxLine);
    u8g2.drawStr(2, 63, txLine);

    if (lastRxMs != 0 && millis() - lastRxMs < 600) {
        u8g2.drawDisc(123, 55, 2);
    }
    if (lastTxMs != 0 && millis() - lastTxMs < 600) {
        u8g2.drawCircle(123, 62, 2);
    }
}

// Half-duplex TX: stop listening, transmit, then resume listening
static bool waitForAck(uint16_t seq) {
    uint32_t start = millis();
    while (millis() - start < CHAT_ACK_WAIT_MS) {
        if (!activeRadio->available()) {
            delayMicroseconds(150);
            continue;
        }

        ChatPacket pkt;
        activeRadio->read(&pkt, sizeof(pkt));
        if (pkt.magic == 0xC7 && pkt.type == 2 && pkt.seq == seq) {
            return true;
        }
        if (pkt.magic == 0xC7 && pkt.type == 1) {
            uint8_t from = pkt.fromRole <= 1 ? pkt.fromRole : 0;
            if (pkt.seq != lastRxSeq[from]) {
                lastRxSeq[from] = pkt.seq;
                copyText(lastInbox, sizeof(lastInbox), pkt.text);
                rxCount++;
                lastRxMs = millis();
            }
        }
    }
    return false;
}

static void sendAck(uint16_t seq) {
    ChatPacket ack;
    memset(&ack, 0, sizeof(ack));
    ack.magic = 0xC7;
    ack.type = 2;
    ack.seq = seq;
    ack.fromRole = roleSelection;

    // Half-duplex: switch to TX, send ACK, switch back to RX
    activeRadio->stopListening();
    activeRadio->flush_tx();
    activeRadio->openWritingPipe(peerAddress());
    activeRadio->write(&ack, sizeof(ack));
    activeRadio->openReadingPipe(1, ownAddress());
    activeRadio->startListening();
}

static void sendSelectedMessage() {
    ChatPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.magic = 0xC7;
    pkt.type = 1;
    pkt.seq = ++txSeq;
    pkt.fromRole = roleSelection;
    pkt.msgId = selectedMessage;
    copyText(pkt.text, sizeof(pkt.text), CHAT_MESSAGES[selectedMessage]);
    copyText(lastOutbox, sizeof(lastOutbox), CHAT_MESSAGES[selectedMessage]);

    lastAckOk = false;
    txCount++;
    lastTxMs = millis();

    for (uint8_t attempt = 0; attempt < 3; attempt++) {
        // Half-duplex: stop listening for TX, then resume
        activeRadio->stopListening();
        activeRadio->flush_tx();
        if (attempt == 0) activeRadio->flush_rx();
        activeRadio->openWritingPipe(peerAddress());
        activeRadio->write(&pkt, sizeof(pkt));
        activeRadio->openReadingPipe(1, ownAddress());
        activeRadio->startListening();

        if (waitForAck(pkt.seq)) {
            lastAckOk = true;
            ackCount++;
            return;
        }
    }
}

// Timed RX window: service incoming packets for a bounded duration
static void serviceIncoming() {
    uint32_t start = millis();
    while (activeRadio->available() && millis() - start < 4) {
        ChatPacket pkt;
        activeRadio->read(&pkt, sizeof(pkt));
        if (pkt.magic != 0xC7) continue;

        if (pkt.type == 1) {
            uint8_t from = pkt.fromRole <= 1 ? pkt.fromRole : 0;
            if (pkt.seq != lastRxSeq[from]) {
                lastRxSeq[from] = pkt.seq;
                copyText(lastInbox, sizeof(lastInbox), pkt.text);
                rxCount++;
                lastRxMs = millis();
            }
            sendAck(pkt.seq);
        } else if (pkt.type == 2 && pkt.seq == txSeq) {
            lastAckOk = true;
        }
    }
}

void nrfChatEnter() {
    screen = CHAT_SETUP;
    roleSelection = 0;
    fieldSelection = 0;
    channelSelection = 4;
    radioReady = false;
    radioBeginOk = false;
    resetChatState();
    Input.resetAll();
}

void nrfChatExit() {
    activeRadio->stopListening();
    activeRadio->powerDown();
    radioReady = false;
    screen = CHAT_SETUP;
    resetChatState();
}

void nrfChatLoop() {
    if (screen == CHAT_SETUP) {
        if (Input.repeating(BTN_ID_UP)) {
            if (fieldSelection == 0) {
                roleSelection = roleSelection == 0 ? 1 : 0;
            } else {
                channelSelection = (channelSelection + 1) % (sizeof(CHAT_CHANNELS) / sizeof(CHAT_CHANNELS[0]));
            }
        }
        if (Input.repeating(BTN_ID_DOWN)) {
            if (fieldSelection == 0) {
                roleSelection = roleSelection == 0 ? 1 : 0;
            } else {
                uint8_t count = sizeof(CHAT_CHANNELS) / sizeof(CHAT_CHANNELS[0]);
                channelSelection = (channelSelection + count - 1) % count;
            }
        }
        if (Input.pressed(BTN_ID_AUX)) {
            fieldSelection = (fieldSelection + 1) % 2;
            Input.consume(BTN_ID_AUX);
        }
        if (Input.pressed(BTN_ID_OK)) {
            resetChatState();
            configureRadio();
            screen = CHAT_ACTIVE;
            Input.consume(BTN_ID_OK);
        }

        u8g2.clearBuffer();
        drawSetup();
        u8g2.sendBuffer(); oledMirrorSync();
        return;
    }

    if (!radioReady) configureRadio();

    // NRF recovery check
    safeNrfRecovery(activeRadio, 0, false);

    serviceIncoming();

    if (Input.repeating(BTN_ID_DOWN)) {
        selectedMessage = (selectedMessage + 1) % CHAT_MESSAGE_COUNT;
    }
    if (Input.repeating(BTN_ID_UP)) {
        selectedMessage = (selectedMessage + CHAT_MESSAGE_COUNT - 1) % CHAT_MESSAGE_COUNT;
    }
    if (Input.pressed(BTN_ID_OK)) {
        sendSelectedMessage();
        lastDrawMs = 0;
        Input.consume(BTN_ID_OK);
    }

    serviceIncoming();

    if (shouldDrawNow()) {
        u8g2.clearBuffer();
        drawChat();
        u8g2.sendBuffer(); oledMirrorSync();
    }
}
