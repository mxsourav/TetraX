#include "app_ir_remote.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include "input_manager.h"
#include "gui_helper.h"
#include "ir_manager.h"
#include "ui_theme.h"
#include "oled_mirror.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool runningApp;

enum RemoteState {
    STATE_TYPE_SELECT,
    STATE_BRAND_SELECT,
    STATE_REMOTE_UI
};

static RemoteState state = STATE_TYPE_SELECT;
static uint8_t selectedType = 0; // 0 = TV, 1 = AC
static uint8_t selectedBrand = 0;
static uint8_t selectedButton = 0;
static bool isTransmitting = false;
static uint32_t txAnimStart = 0;

static const char* TV_BTN_LABELS[] = { "POWER", "VOL+", "VOL-", "CH+", "CH-", "MUTE", "HOME", "BACK" };
static const char* AC_BTN_LABELS[] = { "POWER", "TEMP+", "TEMP-", "FAN", "MODE", "SWING" };

void irRemoteEnter() {
    state = STATE_TYPE_SELECT;
    selectedType = 0;
    selectedBrand = 0;
    selectedButton = 0;
    isTransmitting = false;
    
    // STRICT ISOLATION: Ensure RX is completely off, TX is ready
    IRManager::getInstance().stopReceive();
    
    Serial.println("[IR REMOTE] Entered");
}

void irRemoteExit() {
    Serial.println("[IR REMOTE] Exited");
}

#define DEBUG_LAYOUT 0

static void drawDebugBox(int x, int y, int w, int h) {
#if DEBUG_LAYOUT
    int color = u8g2.getDrawColor();
    u8g2.setDrawColor(2); // XOR mode for debug
    u8g2.drawFrame(x, y, w, h);
    u8g2.setDrawColor(color);
#endif
}

static void drawTypeSelect() {
    u8g2.clearBuffer();
    UiTheme::drawHeader(u8g2, "REMOTE TYPE", "");
    
    int fontAscent = u8g2.getAscent();
    int fontDescent = u8g2.getDescent();
    int fontHeight = fontAscent - fontDescent;
    
    int rowHeight = 22; // Make it taller for the icons
    int startY = 16;    // Start below header
    
    const char* options[] = { "TV REMOTE", "AC REMOTE" };
    for(int i=0; i<2; i++) {
        int rowY = startY + (i * rowHeight);
        
        // Clipping protection
        if (rowY >= 64) continue;
        
        if (i == selectedType) {
            u8g2.drawBox(4, rowY, 120, rowHeight);
            u8g2.setDrawColor(0);
        }
        
        drawDebugBox(4, rowY, 120, rowHeight);
        
        // Draw icon centered vertically in row
        if (i == 0) drawTvRemoteIcon(6, rowY - 6); 
        else drawAcRemoteIcon(6, rowY - 6);
        
        // Text vertically centered
        int textY = rowY + (rowHeight / 2) + (fontAscent / 2) - 1;
        u8g2.drawStr(36, textY, options[i]);
        u8g2.setDrawColor(1);
    }
    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
}

static void drawBrandSelect() {
    u8g2.clearBuffer();
    UiTheme::drawHeader(u8g2, "SELECT BRAND", "");
    
    uint8_t count = (selectedType == 0) ? IRManager::getInstance().getTvBrandCount() : IRManager::getInstance().getAcBrandCount();
    
    int fontAscent = u8g2.getAscent();
    int fontDescent = u8g2.getDescent();
    int rowHeight = (fontAscent - fontDescent) + 6;
    int startY = 16;
    
    int visibleRows = (64 - startY) / rowHeight;
    int startIdx = max(0, min((int)selectedBrand - (visibleRows/2), (int)count - visibleRows));
    if (startIdx < 0) startIdx = 0;
    int endIdx = min((int)count, startIdx + visibleRows);
    
    for(int i=startIdx; i<endIdx; i++) {
        int displayRow = i - startIdx;
        int rowY = startY + (displayRow * rowHeight);
        
        const char* name = (selectedType == 0) ? IRManager::getInstance().getTvBrandName(i) : IRManager::getInstance().getAcBrandName(i);
        
        if (i == selectedBrand) {
            u8g2.drawBox(4, rowY, 120, rowHeight);
            u8g2.setDrawColor(0);
        }
        
        drawDebugBox(4, rowY, 120, rowHeight);
        
        int textY = rowY + (rowHeight / 2) + (fontAscent / 2) - 1;
        u8g2.drawStr(12, textY, name);
        u8g2.setDrawColor(1);
    }
    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
}

static void drawRemoteUI() {
    u8g2.clearBuffer();
    
    const char* brandName = (selectedType == 0) ? IRManager::getInstance().getTvBrandName(selectedBrand) : IRManager::getInstance().getAcBrandName(selectedBrand);
    UiTheme::drawHeader(u8g2, brandName, "");

    if (isTransmitting && (millis() - txAnimStart < 200)) {
        u8g2.drawCircle(115, 8, 4);
        u8g2.drawDisc(115, 8, 2);
    } else {
        isTransmitting = false;
    }

    uint8_t count = (selectedType == 0) ? 8 : 6;
    const char** labels = (selectedType == 0) ? TV_BTN_LABELS : AC_BTN_LABELS;

    int fontAscent = u8g2.getAscent();
    int fontDescent = u8g2.getDescent();
    int rowHeight = (fontAscent - fontDescent) + 6;
    int startY = 16;
    
    int visibleRows = (64 - startY) / rowHeight;
    int startIdx = max(0, min((int)selectedButton - (visibleRows/2), (int)count - visibleRows));
    if (startIdx < 0) startIdx = 0;
    int endIdx = min((int)count, startIdx + visibleRows);

    for(int i=startIdx; i<endIdx; i++) {
        int displayRow = i - startIdx;
        int rowY = startY + (displayRow * rowHeight);
        
        if (i == selectedButton) {
            u8g2.drawBox(8, rowY, 112, rowHeight);
            u8g2.setDrawColor(0);
            
            if (isTransmitting) {
                u8g2.drawFrame(6, rowY-2, 116, rowHeight+4);
            }
        }
        
        drawDebugBox(8, rowY, 112, rowHeight);
        
        int textY = rowY + (rowHeight / 2) + (fontAscent / 2) - 1;
        int width = u8g2.getStrWidth(labels[i]);
        int textX = (128 - width) / 2;
        u8g2.drawStr(textX, textY, labels[i]);
        u8g2.setDrawColor(1);
    }
    
    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
}

void irRemoteLoop() {
    if (state == STATE_TYPE_SELECT) {
        if (Input.pressed(BTN_ID_UP)) {
            if (selectedType > 0) selectedType--;
        }
        if (Input.pressed(BTN_ID_DOWN)) {
            if (selectedType < 1) selectedType++;
        }
        if (Input.pressed(BTN_ID_OK)) {
            state = STATE_BRAND_SELECT;
            selectedBrand = 0;
            delay(150);
        }
        if (Input.pressed(BTN_ID_BACK)) {
            runningApp = false;
            return;
        }
        drawTypeSelect();

    } else if (state == STATE_BRAND_SELECT) {
        uint8_t count = (selectedType == 0) ? IRManager::getInstance().getTvBrandCount() : IRManager::getInstance().getAcBrandCount();
        if (Input.pressed(BTN_ID_UP)) {
            if (selectedBrand > 0) selectedBrand--;
            else selectedBrand = count - 1;
        }
        if (Input.pressed(BTN_ID_DOWN)) {
            if (selectedBrand < count - 1) selectedBrand++;
            else selectedBrand = 0;
        }
        if (Input.pressed(BTN_ID_OK)) {
            state = STATE_REMOTE_UI;
            selectedButton = 0;
            delay(150);
        }
        if (Input.pressed(BTN_ID_BACK)) {
            state = STATE_TYPE_SELECT;
            delay(150);
        }
        drawBrandSelect();

    } else if (state == STATE_REMOTE_UI) {
        uint8_t count = (selectedType == 0) ? 8 : 6;
        if (Input.pressed(BTN_ID_UP)) {
            if (selectedButton > 0) selectedButton--;
            else selectedButton = count - 1;
        }
        if (Input.pressed(BTN_ID_DOWN)) {
            if (selectedButton < count - 1) selectedButton++;
            else selectedButton = 0;
        }
        
        if (Input.repeating(BTN_ID_OK)) {
            bool success = false;
            if (selectedType == 0) {
                success = IRManager::getInstance().transmitTvCommand(selectedBrand, selectedButton);
            } else {
                success = IRManager::getInstance().transmitAcCommand(selectedBrand, selectedButton);
            }
            if (success) {
                isTransmitting = true;
                txAnimStart = millis();
            }
        }

        if (Input.pressed(BTN_ID_BACK)) {
            state = STATE_BRAND_SELECT;
            delay(150);
        }
        
        static uint32_t lastDraw = 0;
    if (millis() - lastDraw > 50) {
        drawRemoteUI();
        lastDraw = millis();
    }
    }
}
