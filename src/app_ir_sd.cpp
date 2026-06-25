#include "app_ir_sd.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include "input_manager.h"
#include "gui_helper.h"
#include "sd_manager.h"
#include "ui_theme.h"
#include "oled_mirror.h"
#include "ir_manager.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool runningApp;

static bool sdMounted = false;

void irSdEnter() {
    sdMounted = SDManager::getInstance().isReady();
    
    // STRICT ISOLATION: Ensure RX is completely off globally
    IRManager::getInstance().stopReceive();
    
    Serial.println("[FROM SD CARD] Entered");
    Serial.println("[IR RX DISABLED]");
}

void irSdExit() {
    Serial.println("[FROM SD CARD] Exited");
}

void irSdLoop() {
    if (Input.pressed(BTN_ID_BACK)) {
        runningApp = false;
        return;
    }

    u8g2.clearBuffer();
    UiTheme::drawHeader(u8g2, "SAVED SIGNALS", "");

    if (!sdMounted) {
        drawFromSdCardIcon(56, 20);
        
        u8g2.setFont(u8g2_font_6x10_tr);
        const char* msg1 = "SD CARD NOT MOUNTED";
        const char* msg2 = "Press BACK to exit";
        
        int w1 = u8g2.getStrWidth(msg1);
        int w2 = u8g2.getStrWidth(msg2);
        
        u8g2.drawStr((128-w1)/2, 50, msg1);
        u8g2.drawStr((128-w2)/2, 62, msg2);
    } else {
        u8g2.setFont(u8g2_font_6x10_tr);
        const char* msg1 = "No .ir files found.";
        const char* msg2 = "(Parser coming soon)";
        int w1 = u8g2.getStrWidth(msg1);
        int w2 = u8g2.getStrWidth(msg2);
        u8g2.drawStr((128-w1)/2, 40, msg1);
        u8g2.drawStr((128-w2)/2, 55, msg2);
    }

    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
    delay(20);
}
