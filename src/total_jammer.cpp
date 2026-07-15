#include "total_jammer.h"
#include "oled_mirror.h"
#include "nrf_helper.h"
#include <U8g2lib.h>
#include "ui_theme.h"

extern U8G2 u8g2;
extern bool isTotalAttacking;

#define BTN_OK 32
#define BTN_BACK 25

byte full_hop_list[] = { 32, 34, 46, 48, 50, 52, 0, 1, 2, 4, 6, 8, 22, 24, 26, 28, 30, 74, 76, 78, 80, 82, 84, 86, 5, 30, 55, 10, 35, 60, 15, 40, 65 };
int total_chans = sizeof(full_hop_list);

static void drawTotalIdle() {
    UiTheme::drawHeader(u8g2, "BARRIDO TOTAL", "READY");
    u8g2.drawRFrame(10, 23, 108, 27, 3);
    u8g2.setFont(u8g2_font_6x10_tr);
    UiTheme::drawCenteredText(u8g2, 34, "WIFI + BT");
    u8g2.setFont(u8g2_font_5x7_tr);
    UiTheme::drawCenteredText(u8g2, 45, "ESPECTRO LISTO");
    UiTheme::drawMiniWave(u8g2, 10, 62, millis() / 90);
    UiTheme::drawMiniWave(u8g2, 108, 62, (millis() / 90) + 6);
}

static void drawTotalActive() {
    uint8_t frame = (millis() / 70) & 0xFF;
    UiTheme::drawHeader(u8g2, "BARRIDO TOTAL", "ON");

    for (int i = 0; i < 16; i++) {
        int x = i * 8;
        int h = 4 + ((frame + i * 5) % 23);
        u8g2.drawVLine(x + ((frame + i) % 3), 60 - h, h);
    }

    u8g2.drawFrame(13, 24, 102, 15);
    u8g2.setFont(u8g2_font_6x10_tr);
    UiTheme::drawCenteredText(u8g2, 35, "ESPECTRO ACTIVO");
}

void totalJammerSetup() {
    activeRadio->begin();
    activeRadio->setAutoAck(false);
    activeRadio->setDataRate(RF24_2MBPS);
    activeRadio->setCRCLength(RF24_CRC_DISABLED);
}

void totalJammerLoop() {
    if (digitalRead(BTN_OK) == LOW) {
        isTotalAttacking = !isTotalAttacking;
        if (!isTotalAttacking) {
            activeRadio->stopConstCarrier();
        } else {
            activeRadio->startConstCarrier(RF24_PA_MAX, 45);
        }
        delay(400);
    }

    if (digitalRead(BTN_BACK) == LOW) {
        isTotalAttacking = false;
        activeRadio->stopConstCarrier();
        return;
    }

    if (isTotalAttacking) {
        unsigned long lastRecovery = millis();
        while (isTotalAttacking) {
            for (int r = 0; r < 50; r++) {
                for (int i = 0; i < total_chans; i++) {
                    activeRadio->setChannel(full_hop_list[i]);
                }

                if (digitalRead(BTN_OK) == LOW || digitalRead(BTN_BACK) == LOW) {
                    isTotalAttacking = false;
                    activeRadio->stopConstCarrier();
                    return;
                }
            }

            // NRF recovery check during aggressive hopping
            if (millis() - lastRecovery > 2000) {
                lastRecovery = millis();
                safeNrfRecovery(activeRadio, 45, true);
            }

            u8g2.clearBuffer();
            drawTotalActive();
            u8g2.sendBuffer(); oledMirrorSync();
        }
    } else {
        u8g2.clearBuffer();
        drawTotalIdle();
        u8g2.sendBuffer(); oledMirrorSync();
    }
}
