#include "app_ir_jammer.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include "input_manager.h"
#include "gui_helper.h"
#include "ir_manager.h"
#include "ui_theme.h"
#include "oled_mirror.h"

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool runningApp;

enum JammerState {
    STATE_JAMMER_SELECT,
    STATE_JAMMER_ACTIVE
};

static JammerState state = STATE_JAMMER_SELECT;
static uint8_t selectedMode = 0; // 0 = TV, 1 = AC
static uint32_t sessionStartTime = 0;

#define GRAPH_WIDTH 128
static uint8_t graphBuffer[GRAPH_WIDTH];

void irJammerEnter() {
    state = STATE_JAMMER_SELECT;
    selectedMode = 0;
    IRManager::getInstance().resetJamStats();
    memset(graphBuffer, 0, sizeof(graphBuffer));
    
    // STRICT ISOLATION: Ensure RX is completely off globally
    IRManager::getInstance().stopReceive();
    
    Serial.println("[IR JAMMER] Entered");
}

void irJammerExit() {
    IRManager::getInstance().emergencyStop();
    Serial.println("[IR JAMMER] Exited");
}

static void drawModeSelect() {
    u8g2.clearBuffer();
    UiTheme::drawHeader(u8g2, "IR JAMMER", "");
    
    int fontAscent = u8g2.getAscent();
    int fontDescent = u8g2.getDescent();
    int rowHeight = (fontAscent - fontDescent) + 6;
    int startY = 16;
    
    const char* options[] = { "TV CARRIER (38kHz)", "AC PROTOCOL FLOOD" };
    for(int i=0; i<2; i++) {
        int rowY = startY + (i * rowHeight);
        
        if (i == selectedMode) {
            u8g2.drawBox(4, rowY, 120, rowHeight);
            u8g2.setDrawColor(0);
        }
        
        int textY = rowY + (rowHeight / 2) + (fontAscent / 2) - 1;
        u8g2.drawStr(8, textY, options[i]);
        u8g2.setDrawColor(1);
    }
    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
}

static void updateGraphData() {
    for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
        if (graphBuffer[i+1] > 2) {
            graphBuffer[i] = graphBuffer[i + 1] - 1;
        } else {
            graphBuffer[i] = graphBuffer[i + 1];
        }
    }
    
    uint8_t newSample = 0;
    if (selectedMode == 0) {
        if (random(100) > 30) {
            newSample = random(12, 22);
        } else {
            newSample = random(1, 4);
        }
    } else {
        static uint8_t burstLen = 0;
        static bool inBurst = false;
        
        if (burstLen == 0) {
            inBurst = !inBurst;
            burstLen = inBurst ? random(8, 15) : random(3, 6);
        }
        burstLen--;
        newSample = inBurst ? random(16, 22) : 0;
    }
    
    graphBuffer[GRAPH_WIDTH - 1] = newSample;
}

static void drawActiveJammer() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tr);
    
    // Header
    u8g2.drawStr(2, 8, "IR JAMMER");
    const char* modeTxt = (selectedMode == 0) ? "38kHz" : "AC FLOOD";
    int w = u8g2.getStrWidth(modeTxt);
    u8g2.drawStr(126 - w, 8, modeTxt);
    u8g2.drawHLine(0, 10, 128); 
    
    // Center Graph Area (y=12 to y=42, height 30px)
    int graphYBase = 40;
    for (int i = 0; i < GRAPH_WIDTH; i++) {
        uint8_t val = graphBuffer[i];
        if (val > 0) {
            if (val > 28) val = 28;
            u8g2.drawVLine(i, graphYBase - val, val);
        }
    }
    
    // Footer
    u8g2.drawHLine(0, 44, 128);
    u8g2.setFont(u8g2_font_4x6_tr);
    
    uint32_t uptime = (millis() - sessionStartTime) / 1000;
    uint32_t mins = uptime / 60;
    uint32_t secs = uptime % 60;
    
    char buf[64];
    const char* strategy = (selectedMode == 0) ? "RAND-OOK" : "AC PROTO";
    snprintf(buf, sizeof(buf), "PKT:%lu D:%d%% %s", 
             IRManager::getInstance().getJamPacketsSent(), 
             IRManager::getInstance().getJamDutyCycle(),
             strategy);
    u8g2.drawStr(2, 53, buf);
    
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02lu:%02lu", mins, secs);
    w = u8g2.getStrWidth(timeBuf);
    u8g2.drawStr(126 - w, 53, timeBuf);
    
    u8g2.sendBuffer(); oledMirrorSync();
    oledMirrorSync();
}

void irJammerLoop() {
    // 1. Process Input Globally FIRST (Highest Priority)
    Input.update();
    
    if (state == STATE_JAMMER_SELECT) {
        if (Input.pressed(BTN_ID_UP)) {
            if (selectedMode > 0) selectedMode--;
        }
        if (Input.pressed(BTN_ID_DOWN)) {
            if (selectedMode < 1) selectedMode++;
        }
        if (Input.pressed(BTN_ID_OK)) {
            Serial.println("[JAMMER] Entering active state");
            Serial.println("[IR_TX_START]");
            state = STATE_JAMMER_ACTIVE;
            IRManager::getInstance().resetJamStats();
            memset(graphBuffer, 0, sizeof(graphBuffer));
            sessionStartTime = millis();
            
            if (selectedMode == 0) {
                IRManager::getInstance().beginTvJammer();
            }
            
            drawActiveJammer();
            delay(150);
        }
        if (Input.pressed(BTN_ID_BACK) || Input.held(BTN_ID_BACK)) {
            Serial.println("[BACK INTERRUPT] Exiting App");
            IRManager::getInstance().emergencyStop();
            runningApp = false;
            return;
        }
        
        static uint32_t lastDraw = 0;
        if (millis() - lastDraw > 50) {
            drawModeSelect();
            lastDraw = millis();
        }
        
    } else if (state == STATE_JAMMER_ACTIVE) {
        // Absolute highest priority exit check
        if (Input.pressed(BTN_ID_BACK) || Input.held(BTN_ID_BACK)) {
            Serial.println("[BACK INTERRUPT] Stopping Carrier");
            Serial.println("[IR_TX_STOP]");
            IRManager::getInstance().emergencyStop();
            state = STATE_JAMMER_SELECT;
            delay(150);
            return;
        }

        // --- HARDWARE TRANSMISSION (MICRO-BURST / PROTOCOL) ---
        if (selectedMode == 0) {
            IRManager::getInstance().jammerTickTv();
        } else {
            IRManager::getInstance().jammerTickAc();
        }
        
        // Ensure space after every burst loop as a safety net
        IRManager::getInstance().emergencyStop();

        // --- INTERNAL GRAPH SAMPLING (Fast) ---
        static uint32_t lastSample = 0;
        if (millis() - lastSample > 30) {
            updateGraphData();
            lastSample = millis();
        }

        // --- VISUALIZATION (~4-5 FPS = 200-250ms) ---
        static uint32_t lastDraw = 0;
        if (millis() - lastDraw > 250) {
            Serial.println("[OLED FRAME] Throttled draw");
            drawActiveJammer();
            lastDraw = millis();
        }
        
        Serial.println("[JAMMER LOOP OK]");
        
        // Cooperative yield
        vTaskDelay(1); 
    }
}
