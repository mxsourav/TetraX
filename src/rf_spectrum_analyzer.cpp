#include "rf_spectrum_analyzer.h"
#include "nrf_helper.h"
#include "input_manager.h"
#include "ui_theme.h"
#include "oled_mirror.h"
#include <U8g2lib.h>
#include <WiFi.h>
#include <string.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

enum class AnalyzerMode {
    MODE_A_SPECTRUM,
    MODE_B_PACKETS,
    MODE_C_SWEEP_TRACK,
    MODE_D_LATENCY
};

static AnalyzerMode currentMode = AnalyzerMode::MODE_A_SPECTRUM;

struct BlueBoxTestPacket {
    char signature[16];
    uint8_t channel;
    uint32_t timestamp;
    uint32_t packetCount;
};

// ============================================================
// CANDLESTICK CHART ENGINE
// ============================================================

#define NUM_CANDLES    42
#define CANDLE_W       2
#define CANDLE_GAP     1
#define CANDLE_STEP    (CANDLE_W + CANDLE_GAP)  // 3px per candle

#define CHART_LEFT     0
#define CHART_TOP      8
#define CHART_BOTTOM   48
#define CHART_HEIGHT   (CHART_BOTTOM - CHART_TOP)  // 40px

#define FOOTER_Y       50

struct Candle {
    uint8_t open;
    uint8_t high;
    uint8_t low;
    uint8_t close;
};

static Candle candles[NUM_CANDLES];

// Per-sweep RF activity accumulator
static const uint8_t MAX_CHANNELS = 126;
static uint16_t sweepAccum = 0;

// Per-frame OHLC tracking (built from sweep snapshots)
static uint8_t frameOpen = 0;
static uint8_t frameHigh = 0;
static uint8_t frameLow = 255;
static uint8_t frameClose = 0;
static bool hadSweepThisFrame = false;

// Scan state
static uint8_t currentScanChannel = 0;
static uint32_t lastRenderMs = 0;
static uint32_t lastSecondTick = 0;

// Telemetry
static uint32_t rxPackets = 0;
static uint32_t rxPacketsLastSecond = 0;
static uint32_t currentPps = 0;
static uint32_t channelsScanned = 0;
static uint32_t scanRate = 0;
static uint32_t framesRendered = 0;
static uint32_t fps = 0;
static uint32_t lastLatency = 0;
static uint8_t peakChannel = 0;
static uint16_t peakHitThisFrame = 0;

static void changeMode(int delta) {
    int m = (int)currentMode + delta;
    if (m > (int)AnalyzerMode::MODE_D_LATENCY) m = 0;
    if (m < 0) m = (int)AnalyzerMode::MODE_D_LATENCY;
    currentMode = (AnalyzerMode)m;
    memset(candles, 0, sizeof(candles));
    rxPackets = 0;
}

void rfSpectrumAnalyzerEnter() {
    WiFi.mode(WIFI_OFF);

    activeRadio->begin();
    activeRadio->setAutoAck(false);
    activeRadio->setDataRate(RF24_2MBPS);
    activeRadio->setPALevel(RF24_PA_MAX);
    activeRadio->startListening();

    memset(candles, 0, sizeof(candles));
    sweepAccum = 0;
    frameOpen = 0; frameHigh = 0; frameLow = 255; frameClose = 0;
    hadSweepThisFrame = false;
    rxPackets = 0;
    lastLatency = 0;
    currentMode = AnalyzerMode::MODE_A_SPECTRUM;
    currentScanChannel = 0;
    Input.resetAll();
    lastRenderMs = millis();
    lastSecondTick = millis();
}

void rfSpectrumAnalyzerExit() {
    activeRadio->stopListening();
}

// Scale raw sweep total (0..~600) to chart pixel height (0..CHART_HEIGHT)
static uint8_t scaleToChart(uint16_t raw) {
    if (raw > 500) raw = 500;
    uint16_t scaled = (raw * CHART_HEIGHT) / 500;
    if (scaled > CHART_HEIGHT) scaled = CHART_HEIGHT;
    return (uint8_t)scaled;
}

// ============================================================
// RF SCAN LOOP - runs as fast as possible
// ============================================================
static void rfScanLoop() {
    // Per-second telemetry
    if (millis() - lastSecondTick > 1000) {
        lastSecondTick = millis();
        currentPps = rxPacketsLastSecond;
        rxPacketsLastSecond = 0;
        scanRate = channelsScanned;
        channelsScanned = 0;
        fps = framesRendered;
        framesRendered = 0;
    }

    if (currentMode == AnalyzerMode::MODE_A_SPECTRUM || currentMode == AnalyzerMode::MODE_C_SWEEP_TRACK) {
        for (int step = 0; step < 12; step++) {
            activeRadio->setChannel(currentScanChannel);
            delayMicroseconds(130);
            channelsScanned++;

            bool carrier = activeRadio->testCarrier();
            bool rpd = activeRadio->testRPD();

            uint16_t hit = 0;
            if (rpd) hit += 12;
            if (carrier) hit += 4;

            if (activeRadio->available()) {
                char buf[32];
                activeRadio->read(buf, 32);
                BlueBoxTestPacket* pkt = (BlueBoxTestPacket*)buf;
                if (strncmp(pkt->signature, "BLUEBOX_TEST_V1", 15) == 0) {
                    hit += 25;
                    rxPackets++;
                    rxPacketsLastSecond++;
                }
            }

            sweepAccum += hit;

            // Track peak channel for footer
            if (hit > peakHitThisFrame) {
                peakHitThisFrame = hit;
                peakChannel = currentScanChannel;
            }

            currentScanChannel++;
            if (currentScanChannel >= MAX_CHANNELS) {
                currentScanChannel = 0;

                // === END OF ONE FULL SWEEP ===
                uint8_t level = scaleToChart(sweepAccum);

                if (!hadSweepThisFrame) {
                    // First sweep this frame: set open
                    frameOpen = level;
                    frameHigh = level;
                    frameLow = level;
                    hadSweepThisFrame = true;
                }
                if (level > frameHigh) frameHigh = level;
                if (level < frameLow) frameLow = level;
                frameClose = level;  // always update close

                sweepAccum = 0;
            }
        }
    } else if (currentMode == AnalyzerMode::MODE_B_PACKETS || currentMode == AnalyzerMode::MODE_D_LATENCY) {
        activeRadio->setChannel(42);
        delayMicroseconds(130);
        channelsScanned++;
        if (activeRadio->available()) {
            char buf[32];
            activeRadio->read(buf, 32);
            BlueBoxTestPacket* pkt = (BlueBoxTestPacket*)buf;
            if (strncmp(pkt->signature, "BLUEBOX_TEST_V1", 15) == 0) {
                rxPackets++;
                rxPacketsLastSecond++;
                if (currentMode == AnalyzerMode::MODE_D_LATENCY) {
                    lastLatency = millis() - pkt->timestamp;
                }
            }
        }
    }
}

// ============================================================
// DRAWING HELPERS
// ============================================================

// Draw dotted horizontal line
static void drawDottedHLine(uint8_t y, uint8_t x0, uint8_t x1) {
    for (uint8_t x = x0; x < x1; x += 3) {
        u8g2.drawPixel(x, y);
    }
}

// Draw a single candlestick at pixel position x
static void drawCandle(int x, const Candle& c) {
    if (c.high == 0 && c.low == 0 && c.open == 0 && c.close == 0) return;

    // Convert chart values to screen Y (inverted: higher value = lower Y)
    int yHigh  = CHART_BOTTOM - c.high;
    int yLow   = CHART_BOTTOM - c.low;
    int yOpen  = CHART_BOTTOM - c.open;
    int yClose = CHART_BOTTOM - c.close;

    // Wick: thin 1px line from high to low (centered in candle)
    int wickX = x + CANDLE_W / 2;
    if (yHigh < yLow) {
        u8g2.drawVLine(wickX, yHigh, yLow - yHigh + 1);
    }

    // Body: rectangle between open and close
    int bodyTop = (yOpen < yClose) ? yOpen : yClose;
    int bodyBot = (yOpen > yClose) ? yOpen : yClose;
    int bodyH = bodyBot - bodyTop;
    if (bodyH < 1) bodyH = 1;

    if (c.close >= c.open) {
        // Bullish (price went up): outline only
        u8g2.drawFrame(x, bodyTop, CANDLE_W, bodyH + 1);
    } else {
        // Bearish (price went down): filled
        u8g2.drawBox(x, bodyTop, CANDLE_W, bodyH + 1);
    }
}

// ============================================================
// DRAW MODE A: CANDLESTICK TIME HISTORY
// ============================================================
static void drawModeA() {
    // --- HEADER ---
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(0, 6, "RF ACTIVITY (TIME HISTORY)");

    // Status indicator top-right
    if (currentPps > 0) {
        u8g2.drawStr(100, 6, "RF:ON");
    } else {
        u8g2.drawStr(100, 6, "RF:--");
    }

    // --- CHART FRAME ---
    // Left axis line
    // u8g2.drawVLine(0, CHART_TOP, CHART_HEIGHT);

    // Dotted grid lines at 25%, 50%, 75%
    uint8_t y25 = CHART_BOTTOM - (CHART_HEIGHT / 4);
    uint8_t y50 = CHART_BOTTOM - (CHART_HEIGHT / 2);
    uint8_t y75 = CHART_BOTTOM - (3 * CHART_HEIGHT / 4);

    drawDottedHLine(y25, 0, 128);
    drawDottedHLine(y50, 0, 128);
    drawDottedHLine(y75, 0, 128);

    // Bottom baseline
    u8g2.drawHLine(0, CHART_BOTTOM, 128);

    // --- DRAW CANDLESTICKS ---
    for (int i = 0; i < NUM_CANDLES; i++) {
        int x = CHART_LEFT + i * CANDLE_STEP;
        drawCandle(x, candles[i]);
    }

    // --- CURRENT LEVEL ARROW on right edge ---
    if (hadSweepThisFrame || candles[NUM_CANDLES - 1].close > 0) {
        uint8_t lvl = candles[NUM_CANDLES - 1].close;
        int arrowY = CHART_BOTTOM - lvl;
        if (arrowY >= CHART_TOP && arrowY <= CHART_BOTTOM) {
            // Small arrow: "> " pointing right
            u8g2.drawPixel(125, arrowY);
            u8g2.drawPixel(126, arrowY - 1);
            u8g2.drawPixel(126, arrowY);
            u8g2.drawPixel(126, arrowY + 1);
            u8g2.drawPixel(127, arrowY);
        }
    }

    // --- FOOTER ---
    u8g2.drawHLine(0, FOOTER_Y - 1, 128);

    u8g2.setFont(u8g2_font_4x6_tr);

    char f1[40];
    snprintf(f1, sizeof(f1), "CH:%d PKT:%lu FPS:%lu SPAM:ON", peakChannel, rxPackets, fps);
    u8g2.drawStr(0, FOOTER_Y + 6, f1);

    char f2[40];
    snprintf(f2, sizeof(f2), "SCN:%lu PPS:%lu PWR:MAX 2M", scanRate, currentPps);
    u8g2.drawStr(0, FOOTER_Y + 13, f2);
}

// ============================================================
// OTHER MODES (unchanged)
// ============================================================
static void drawModeB() {
    UiTheme::drawHeader(u8g2, "PACKET CNT", "B");
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(10, 30, "CH: 42 (Locked)");
    char buf[32];
    snprintf(buf, sizeof(buf), "TOTAL: %lu", rxPackets);
    u8g2.drawStr(10, 45, buf);
    snprintf(buf, sizeof(buf), "RATE: %lu PPS", currentPps);
    u8g2.drawStr(10, 60, buf);
}

static void drawModeC() {
    UiTheme::drawHeader(u8g2, "SWEEP TRK", "C");
}

static void drawModeD() {
    UiTheme::drawHeader(u8g2, "LATENCY", "D");
    u8g2.setFont(u8g2_font_6x10_tf);
    char buf[32];
    snprintf(buf, sizeof(buf), "LATENCY: %lu ms", lastLatency);
    u8g2.drawStr(10, 35, buf);
}

// ============================================================
// RENDER LOOP - 30 FPS, shifts candles left, pushes new candle
// ============================================================
static void rfRenderLoop() {
    if (millis() - lastRenderMs < 33) return;
    lastRenderMs = millis();
    framesRendered++;

    // === COMMIT CURRENT CANDLE ===
    Candle newCandle;
    if (hadSweepThisFrame) {
        newCandle.open  = frameOpen;
        newCandle.high  = frameHigh;
        newCandle.low   = frameLow;
        newCandle.close = frameClose;
    } else {
        // No sweep completed this frame - flat candle at previous close
        uint8_t prev = candles[NUM_CANDLES - 1].close;
        newCandle.open = prev;
        newCandle.high = prev;
        newCandle.low  = prev;
        newCandle.close = prev;
    }

    // Shift all candles left by 1
    memmove(candles, candles + 1, sizeof(Candle) * (NUM_CANDLES - 1));
    candles[NUM_CANDLES - 1] = newCandle;

    // Reset frame OHLC for next frame
    frameOpen = 0; frameHigh = 0; frameLow = 255; frameClose = 0;
    hadSweepThisFrame = false;
    peakHitThisFrame = 0;

    // === RENDER ===
    u8g2.clearBuffer();

    switch (currentMode) {
        case AnalyzerMode::MODE_A_SPECTRUM: drawModeA(); break;
        case AnalyzerMode::MODE_B_PACKETS:  drawModeB(); break;
        case AnalyzerMode::MODE_C_SWEEP_TRACK: drawModeC(); break;
        case AnalyzerMode::MODE_D_LATENCY:  drawModeD(); break;
    }

    u8g2.sendBuffer();
    oledMirrorSync();
}

// ============================================================
// MAIN LOOP
// ============================================================
void rfSpectrumAnalyzerLoop() {
    if (Input.pressed(BTN_ID_RIGHT)) {
        changeMode(1);
        Input.consume(BTN_ID_RIGHT);
    } else if (Input.pressed(BTN_ID_LEFT)) {
        changeMode(-1);
        Input.consume(BTN_ID_LEFT);
    }

    rfScanLoop();
    rfRenderLoop();
}
