#include "app_raone_ui.h"
#include "raone_link.h"
#include "input_manager.h"
#include "ui_theme.h"
#include <U8g2lib.h>
#include "Deauther.h" // For local 2.4G deauth

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool runningApp;

enum RaoneMenuState {
    MENU_5G_DEAUTH = 0,
    MENU_5G_BEACON,
    MENU_DUAL_DEAUTH,
    MENU_DUAL_BEACON,
    MENU_REMOTE_CONTROL,
    MENU_STOP,
    MENU_COUNT
};

static int currentState = 0;
static bool attackActive = false;
static bool dualMode = false;

const char* menuItems[] = {
    "1. 5G Deauth (All Ch)",
    "2. 5G Beacon Spam",
    "3. Dual Deauth (2.4G+5G)",
    "4. Dual Beacon Spam",
    "5. Remote Control RAONE",
    "6. Stop Coprocessor"
};

void raoneUiEnter() {
    currentState = 0;
    attackActive = false;
    dualMode = false;

    // If disconnected, try a quick on-demand handshake ping
    if (!RaoneLink::getInstance().isConnected()) {
        RaoneLink::getInstance().performBootHandshake();
    }
}

static void formatMetric(char *out, size_t outSize, uint32_t count) {
    if (count < 10000) {
        snprintf(out, outSize, "%lu", (unsigned long)count);
    } else if (count < 1000000) {
        snprintf(out, outSize, "%.1fk", (float)count / 1000.0f);
    } else {
        snprintf(out, outSize, "%.2fM", (float)count / 1000000.0f);
    }
}

static void drawMenu() {
    u8g2.clearBuffer();
    UiTheme::drawHeader(u8g2, "tetrax", "RAONE 5G");

    if (!RaoneLink::getInstance().isConnected()) {
        u8g2.setFont(u8g2_font_5x7_tr);
        u8g2.drawStr(10, 35, "[5G COPROCESSOR");
        u8g2.drawStr(10, 45, " DISCONNECTED]");
        u8g2.sendBuffer();
        return;
    }

    u8g2.setFont(u8g2_font_5x7_tr);
    for (int i = 0; i < 3; i++) {
        int idx = currentState - 1 + i;
        if (idx >= 0 && idx < MENU_COUNT) {
            if (i == 1) {
                u8g2.setDrawColor(1);
                u8g2.drawBox(0, 18 + i * 12 - 9, 128, 11);
                u8g2.setDrawColor(0);
                u8g2.drawStr(5, 18 + i * 12, menuItems[idx]);
                u8g2.setDrawColor(1);
            } else {
                u8g2.drawStr(5, 18 + i * 12, menuItems[idx]);
            }
        }
    }
    u8g2.sendBuffer();
}

static void drawAttackScreen() {
    u8g2.clearBuffer();
    
    // Title
    u8g2.setFont(u8g2_font_ncenB08_tr);
    if (dualMode) {
        u8g2.drawStr(10, 12, "DUAL-BAND ATTACK");
    } else {
        u8g2.drawStr(10, 12, "RAONE 5G ATTACK");
    }
    
    u8g2.setFont(u8g2_font_5x7_tr);
    
    // Status / Mode
    if (dualMode) {
        u8g2.drawStr(5, 25, "2.4G [ON] + 5G [ON]");
    } else {
        u8g2.drawStr(5, 25, "STATUS: TRANSMITTING");
    }

    char buf[64];
    char txBuf[16];
    char errBuf[16];
    char ppsBuf[16];
    
    formatMetric(txBuf, sizeof(txBuf), RaoneLink::getInstance().getLiveTxCount());
    formatMetric(errBuf, sizeof(errBuf), RaoneLink::getInstance().getFailCount());
    formatMetric(ppsBuf, sizeof(ppsBuf), RaoneLink::getInstance().getLivePps());
    
    // Channel & PPS Line
    int ch = RaoneLink::getInstance().getRaoneChannel();
    if (ch == 0) {
        snprintf(buf, sizeof(buf), "CH: HOP    RATE: %s pkt/s", ppsBuf);
    } else {
        snprintf(buf, sizeof(buf), "CH: %d      RATE: %s pkt/s", ch, ppsBuf);
    }
    u8g2.drawStr(5, 37, buf);
    
    // TX & Error Line
    snprintf(buf, sizeof(buf), "TX: %s   ERR: %s", txBuf, errBuf);
    u8g2.drawStr(5, 49, buf);
    
    // Footer
    u8g2.drawStr(5, 62, "[BACK to Stop Attack]");
    
    u8g2.sendBuffer();
}

static void handleRemoteControl() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(10, 15, "RAONE REMOTE");
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(10, 35, "UP/DN -> NAV");
    u8g2.drawStr(10, 45, "OK -> OK");
    u8g2.drawStr(10, 55, "BACK -> Exit RC");
    u8g2.sendBuffer();

    if (Input.pressed(BTN_ID_DOWN) || Input.pressed(BTN_ID_UP)) {
        RaoneLink::getInstance().sendCommand("CMD:NAV");
        Input.consume(BTN_ID_DOWN); Input.consume(BTN_ID_UP);
    }
    if (Input.pressed(BTN_ID_OK)) {
        RaoneLink::getInstance().sendCommand("CMD:OK");
        Input.consume(BTN_ID_OK);
    }
    if (Input.pressed(BTN_ID_BACK)) {
        attackActive = false; // Exit remote control mode
        Input.consume(BTN_ID_BACK);
    }
}

void raoneUiLoop() {
    if (!RaoneLink::getInstance().isConnected()) {
        drawMenu();
        return;
    }

    if (attackActive) {
        if (currentState == MENU_REMOTE_CONTROL) {
            handleRemoteControl();
        } else {
            drawAttackScreen();
            
            // Need to tick the local 2.4G deauther if dual mode is active
            if (dualMode && currentState == MENU_DUAL_DEAUTH) {
                runDeauther(); // Note: This might block or take over, need to handle carefully depending on runDeauther implementation
            }

            if (Input.pressed(BTN_ID_BACK)) {
                RaoneLink::getInstance().sendCommand("CMD:STOP");
                attackActive = false;
                dualMode = false;
                Input.consume(BTN_ID_BACK);
            }
        }
    } else {
        drawMenu();

        if (Input.repeating(BTN_ID_UP)) {
            currentState--;
            if (currentState < 0) currentState = MENU_COUNT - 1;
        }
        if (Input.repeating(BTN_ID_DOWN)) {
            currentState++;
            if (currentState >= MENU_COUNT) currentState = 0;
        }

        if (Input.pressed(BTN_ID_OK)) {
            Input.consume(BTN_ID_OK);
            switch (currentState) {
                case MENU_5G_DEAUTH:
                    RaoneLink::getInstance().sendCommand("CMD:DEAUTH_ALL_5G");
                    attackActive = true;
                    break;
                case MENU_5G_BEACON:
                    RaoneLink::getInstance().sendCommand("CMD:BEACON_5G");
                    attackActive = true;
                    break;
                case MENU_DUAL_DEAUTH:
                    RaoneLink::getInstance().sendCommand("CMD:DEAUTH_ALL_5G");
                    // Assuming deauther is set up for global sweep
                    dualMode = true;
                    attackActive = true;
                    break;
                case MENU_DUAL_BEACON:
                    RaoneLink::getInstance().sendCommand("CMD:BEACON_5G");
                    // Need to trigger 2.4G beacon spam here
                    dualMode = true;
                    attackActive = true;
                    break;
                case MENU_REMOTE_CONTROL:
                    attackActive = true; // Use this to jump into RC state
                    break;
                case MENU_STOP:
                    RaoneLink::getInstance().sendCommand("CMD:STOP");
                    break;
            }
        }
    }
}

void raoneUiExit() {
    if (attackActive) {
        RaoneLink::getInstance().sendCommand("CMD:STOP");
    }
}




