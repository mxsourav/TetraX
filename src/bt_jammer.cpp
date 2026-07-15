#include "bt_jammer.h"
#include "oled_mirror.h"
#include <U8g2lib.h>
#include "ui_theme.h"
#include "nrf_helper.h"

extern U8G2 u8g2;

#define BTN_OK 32
#define BTN_BACK 25

bool isBtJamming = false;
byte hopping_channel[] = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80 };
int total_bt_chans = sizeof(hopping_channel);

static void drawBtGlyph(int x, int y, uint8_t frame) {
    u8g2.drawLine(x + 8, y + 2, x + 8, y + 26);
    u8g2.drawLine(x + 8, y + 2, x + 18, y + 8);
    u8g2.drawLine(x + 18, y + 8, x + 8, y + 14);
    u8g2.drawLine(x + 8, y + 14, x + 18, y + 20);
    u8g2.drawLine(x + 18, y + 20, x + 8, y + 26);
    u8g2.drawLine(x + 2, y + 8, x + 24, y + 22);
    if ((frame / 4) % 2 == 0) u8g2.drawCircle(x + 8, y + 14, 13);
}

static void drawBtIdle() {
    uint8_t frame = (millis() / 90) & 0xFF;
    UiTheme::drawHeader(u8g2, "BT JAMMER", "READY");
    drawBtGlyph(14, 25, frame);
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(48, 35, "ESPECTRO");
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(50, 47, "BT LISTO");
    UiTheme::drawMiniWave(u8g2, 48, 62, frame);
}

static void drawBtActiveOnce() {
    UiTheme::drawHeader(u8g2, "BT JAMMER", "ON");
    drawBtGlyph(14, 25, millis() / 90);
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(47, 33, "MAX MODE");
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(49, 45, "HOPPING");
    for (int i = 0; i < 8; i++) {
        u8g2.drawBox(48 + i * 9, 62 - (3 + i), 5, 3 + i);
    }
}

void btJammerSetup() {
    activeRadio->begin();
    activeRadio->setAutoAck(false); 
    activeRadio->setPALevel(RF24_PA_MAX, true);
    activeRadio->setDataRate(RF24_1MBPS); 
    activeRadio->setCRCLength(RF24_CRC_DISABLED);
}

void btJammerLoop() {
    if (digitalRead(BTN_OK) == LOW) {
        isBtJamming = !isBtJamming;
        if (!isBtJamming) {
            activeRadio->stopConstCarrier();
        } else {
            u8g2.clearBuffer();
            drawBtActiveOnce();
            u8g2.sendBuffer(); oledMirrorSync();
            activeRadio->startConstCarrier(RF24_PA_MAX, hopping_channel[0]);
        }
        delay(400);
    }

    if (isBtJamming) {
        unsigned long lastCheck = millis();
        while (isBtJamming) {
            for (int i = 0; i < total_bt_chans; i++) {
                activeRadio->setChannel(hopping_channel[i]);
            }
            if (millis() - lastCheck > 1000) {
                lastCheck = millis();
                safeNrfRecovery(activeRadio, hopping_channel[0], true);
            }
            if (digitalRead(BTN_OK) == LOW || digitalRead(BTN_BACK) == LOW) {
                isBtJamming = false; 
                activeRadio->stopConstCarrier(); 
                return;
            }
        }
    } else {
        u8g2.clearBuffer();
        drawBtIdle();
        u8g2.sendBuffer(); oledMirrorSync();
    }
}
