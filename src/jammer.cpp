#include "jammer.h"
#include "oled_mirror.h"
#include "nrf_helper.h"
#include <U8g2lib.h>
#include "ui_theme.h"
#include "input_manager.h"

extern U8G2 u8g2;

#define BTN_UP 26
#define BTN_DOWN 33
#define BTN_OK 32
#define BTN_BACK 25

int jamChannel = 1;
bool isAttacking = false;

const byte noise_payload[] = {0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA};

static void drawChannelGauge(int channel, bool active) {
    char status[8];
    snprintf(status, sizeof(status), "CH:%02d", channel);
    UiTheme::drawHeader(u8g2, "JAMMER CANAL", status);

    u8g2.setFont(u8g2_font_6x12_tr);
    char chText[10];
    snprintf(chText, sizeof(chText), "%02d", channel);
    int tw = u8g2.getStrWidth(chText);
    u8g2.drawStr((128 - tw) / 2, 31, chText);

    int pct = 7 + ((channel - 1) * 93) / 13;
    UiTheme::drawProgressBar(u8g2, 16, 38, 96, 7, pct);

    u8g2.setFont(u8g2_font_5x7_tr);
    if (active) {
        uint8_t frame = (millis() / 80) & 0xFF;
        for (int i = 0; i < 13; i++) {
            uint8_t h = 3 + ((frame + i * 3) % 16);
            u8g2.drawBox(8 + i * 9, 63 - h, 5, h);
        }
        UiTheme::drawCenteredText(u8g2, 53, "ACTIVO");
    } else {
        u8g2.drawFrame(35, 50, 58, 10);
        UiTheme::drawCenteredText(u8g2, 58, "LISTO");
    }
}

void jammerSetup() {
    activeRadio->begin();
    activeRadio->setAddressWidth(3);
    activeRadio->setRetries(0, 0);
    activeRadio->setDataRate(RF24_2MBPS);
    activeRadio->setAutoAck(false);
    activeRadio->stopListening();
}

void jammerExit() {
    isAttacking = false;
    activeRadio->stopConstCarrier();
    Input.consume(BTN_ID_BACK);
}

void jammerLoop() {
    if (Input.repeating(BTN_ID_UP)) {
        if (jamChannel < 14) {
            jamChannel++;
            if (isAttacking) {
                int freq = (jamChannel * 5) + 2;
                activeRadio->startConstCarrier(RF24_PA_MAX, (uint8_t)freq);
            }
        }
    }

    if (Input.repeating(BTN_ID_DOWN)) {
        if (jamChannel > 1) {
            jamChannel--;
            if (isAttacking) {
                int freq = (jamChannel * 5) + 2;
                activeRadio->startConstCarrier(RF24_PA_MAX, (uint8_t)freq);
            }
        }
    }

    if (Input.pressed(BTN_ID_OK)) {
        isAttacking = !isAttacking;
        int freq = (jamChannel * 5) + 2;
        if (isAttacking) {
            activeRadio->startConstCarrier(RF24_PA_MAX, (uint8_t)freq);
        } else {
            activeRadio->stopConstCarrier();
        }
    }

    u8g2.clearBuffer();
    drawChannelGauge(jamChannel, isAttacking);
    u8g2.sendBuffer(); oledMirrorSync();

    if (isAttacking) {
        int freq = (jamChannel * 5) + 2;
        activeRadio->setChannel(freq);
        for (int i = 0; i < 20; i++) {
            activeRadio->startWrite(&noise_payload, sizeof(noise_payload), true);
        }
        // NRF recovery check during aggressive TX
        safeNrfRecovery(activeRadio, freq, true);
    }
}
