#include "app_ir_receiver.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include "input_manager.h"
#include "gui_helper.h"
#include "ir_manager.h"
#include "ui_theme.h"
#include "sd_manager.h"
#include "oled_mirror.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool runningApp;

enum ReceiverState {
    STATE_RX_LISTENING,
    STATE_RX_MENU
};

static ReceiverState rxState = STATE_RX_LISTENING;
static AppIRData lastData;
static uint8_t rxMenuIndex = 0;
static const char* RX_MENU[] = { "SAVE TO SD", "REPLAY SIGNAL", "DISCARD" };

void irReceiverEnter() {
    rxState = STATE_RX_LISTENING;
    rxMenuIndex = 0;
    IRManager::getInstance().startReceive();
    Serial.println("[IR RECEIVER] Entered");
}

void irReceiverExit() {
    IRManager::getInstance().stopReceive();
    Serial.println("[IR RECEIVER] Exited");
}

static void drawListening() {
    u8g2.clearBuffer();
    UiTheme::drawHeader(u8g2, "IR RECEIVER", "");
    
    drawIrReceiverIcon(56, 20);
    
    int dots = (millis() / 500) % 4;
    char buf[16] = "LISTENING";
    for(int i=0; i<dots; i++) strcat(buf, ".");
    
    int w = u8g2.getStrWidth(buf);
    u8g2.drawStr((128-w)/2, 50, buf);
    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
}

static void drawReceivedMenu() {
    u8g2.clearBuffer();
    char header[32];
    snprintf(header, sizeof(header), "P:%d A:%02X C:%02X", lastData.protocol, lastData.address, lastData.command);
    UiTheme::drawHeader(u8g2, "SIGNAL ACQUIRED", header);
    
    bool sdOk = SDManager::getInstance().isReady();

    int fontAscent = u8g2.getAscent();
    int fontDescent = u8g2.getDescent();
    int rowHeight = (fontAscent - fontDescent) + 6;
    int startY = 16;

    for(int i=0; i<3; i++) {
        int rowY = startY + (i * rowHeight);
        
        // Clipping protection
        if (rowY >= 64) continue;
        
        if (i == rxMenuIndex) {
            u8g2.drawBox(8, rowY, 112, rowHeight);
            u8g2.setDrawColor(0);
        }
        
        int textY = rowY + (rowHeight / 2) + (fontAscent / 2) - 1;
        
        if (i == 0 && !sdOk) {
            u8g2.drawStr(16, textY, "SAVE (NO SD)");
        } else {
            u8g2.drawStr(16, textY, RX_MENU[i]);
        }
        
        u8g2.setDrawColor(1);
    }
    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
}

void irReceiverLoop() {
    if (rxState == STATE_RX_LISTENING) {
        if (Input.pressed(BTN_ID_BACK)) {
            runningApp = false;
            return;
        }
        
        if (IRManager::getInstance().checkReceivedSignal(&lastData, nullptr, nullptr)) {
            Serial.println("[IR_RX]");
            rxState = STATE_RX_MENU;
            rxMenuIndex = 1; // Default to REPLAY to avoid accidentally trying to save to a missing SD
        }
        
        drawListening();
    } else if (rxState == STATE_RX_MENU) {
        if (Input.pressed(BTN_ID_UP)) {
            if (rxMenuIndex > 0) rxMenuIndex--;
            else rxMenuIndex = 2;
        }
        if (Input.pressed(BTN_ID_DOWN)) {
            if (rxMenuIndex < 2) rxMenuIndex++;
            else rxMenuIndex = 0;
        }
        if (Input.pressed(BTN_ID_BACK)) {
            rxState = STATE_RX_LISTENING;
            IRManager::getInstance().startReceive();
            delay(150);
        }
        if (Input.pressed(BTN_ID_OK)) {
            if (rxMenuIndex == 0) {
                if (SDManager::getInstance().isReady()) {
                    Serial.println("[IR RECEIVER] Saving to SD...");
                    // Placeholder for SD save implementation
                    delay(300);
                    rxState = STATE_RX_LISTENING;
                    IRManager::getInstance().startReceive();
                } else {
                    // SD not mounted, ignore
                    Serial.println("[IR RECEIVER] Cannot save, SD not available.");
                }
            } else if (rxMenuIndex == 1) {
                Serial.println("[IR RECEIVER] Replaying signal...");
                IRManager::getInstance().replaySignal(&lastData);
                delay(300);
            } else if (rxMenuIndex == 2) {
                rxState = STATE_RX_LISTENING;
                IRManager::getInstance().startReceive();
            }
            delay(150);
        }
        
        drawReceivedMenu();
    }
}
