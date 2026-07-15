#include "rf_heatmap.h"
#include "oled_mirror.h"
#include "input_manager.h"
#include "ui_theme.h"
#include "nrf_helper.h"
#include <U8g2lib.h>
#include <WiFi.h>
#include <string.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

static const uint8_t RF_CHANNELS = 125;
static const uint8_t BUCKET_COUNT = 31;
static const uint8_t HISTORY_ROWS = 9;
static const uint8_t SAMPLES_PER_CHANNEL = 28;

static uint8_t heatRows[HISTORY_ROWS][BUCKET_COUNT];
static uint16_t sweeps = 0;
static uint8_t hotBucket = 0;
static uint8_t hotLevel = 0;
static uint8_t frameTick = 0;

static uint8_t sampleRadioLocal(uint8_t channel) {
    activeRadio->setChannel(channel);
    delayMicroseconds(95);

    uint8_t hits = 0;
    for (uint8_t i = 0; i < SAMPLES_PER_CHANNEL; i++) {
        if (activeRadio->testCarrier()) hits++;
    }
    return hits;
}

static uint8_t levelFromAverage(uint16_t energy, uint8_t samples) {
    if (samples == 0) return 0;
    uint8_t avg = energy / samples;
    if (avg >= 18) return 3;
    if (avg >= 8)  return 2;
    if (avg >= 2)  return 1;
    return 0;
}

static void pushHeatRow(const uint8_t* row) {
    for (uint8_t r = 0; r < HISTORY_ROWS - 1; r++) {
        memcpy(heatRows[r], heatRows[r + 1], BUCKET_COUNT);
    }
    memcpy(heatRows[HISTORY_ROWS - 1], row, BUCKET_COUNT);
}

static void scanHeatRow(uint8_t* outRow) {
    uint16_t bucketEnergy[BUCKET_COUNT];
    uint8_t bucketSamples[BUCKET_COUNT];
    memset(bucketEnergy, 0, sizeof(bucketEnergy));
    memset(bucketSamples, 0, sizeof(bucketSamples));

    // Sequential scan of all 125 channels with single radio
    for (uint8_t ch = 0; ch < RF_CHANNELS; ch++) {
        uint8_t b = min<uint8_t>(ch / 4, BUCKET_COUNT - 1);
        bucketEnergy[b] += sampleRadioLocal(ch);
        bucketSamples[b]++;
    }

    hotBucket = 0;
    hotLevel = 0;
    for (uint8_t b = 0; b < BUCKET_COUNT; b++) {
        outRow[b] = levelFromAverage(bucketEnergy[b], bucketSamples[b]);
        if (outRow[b] >= hotLevel) {
            hotLevel = outRow[b];
            hotBucket = b;
        }
    }
    sweeps++;
    // Periodic recovery check
    if ((sweeps & 0x000F) == 0) safeNrfRecovery(activeRadio, 0, false);
}

static void drawCell(uint8_t x, uint8_t y, uint8_t level) {
    switch (level) {
        case 0:
            if (((x + y + frameTick) & 7) == 0) u8g2.drawPixel(x + 1, y + 1);
            break;
        case 1:
            u8g2.drawPixel(x + 1, y + 1);
            u8g2.drawPixel(x + 2, y + 2);
            break;
        case 2:
            u8g2.drawFrame(x, y, 3, 3);
            u8g2.drawPixel(x + 1, y + 1);
            break;
        default:
            u8g2.drawBox(x, y, 3, 3);
            break;
    }
}

static void drawMarkers() {
    const uint8_t wifiCenters[] = {12, 37, 62};
    const char* labels[] = {"1", "6", "11"};
    u8g2.setFont(u8g2_font_4x6_tf);
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t bucket = wifiCenters[i] / 4;
        uint8_t x = 2 + bucket * 4;
        u8g2.drawVLine(x, 20, 2);
        u8g2.drawStr(constrain((int)x - 2, 0, 122), 63, labels[i]);
    }
    u8g2.drawStr(111, 63, "BT");
}

static void drawHeatmap() {
    char status[12];
    uint8_t hotChannel = min<uint8_t>(hotBucket * 4 + 2, RF_CHANNELS - 1);
    snprintf(status, sizeof(status), "CH%03u", hotChannel);
    UiTheme::drawHeader(u8g2, "RF HEATMAP", status);

    u8g2.setFont(u8g2_font_4x6_tf);
    char meta[22];
    snprintf(meta, sizeof(meta), "SW%04u HOT%u", sweeps, hotLevel);
    u8g2.drawStr(2, 20, meta);
    UiTheme::drawMiniWave(u8g2, 108, 20, frameTick);

    for (uint8_t r = 0; r < HISTORY_ROWS; r++) {
        uint8_t y = 23 + r * 4;
        for (uint8_t b = 0; b < BUCKET_COUNT; b++) {
            drawCell(2 + b * 4, y, heatRows[r][b]);
        }
    }

    drawMarkers();
}

void rfHeatmapEnter() {
    WiFi.mode(WIFI_OFF);

    activeRadio->begin();
    activeRadio->setAutoAck(false);
    activeRadio->setDataRate(RF24_2MBPS);
    activeRadio->setPALevel(RF24_PA_MAX);
    activeRadio->startListening();

    memset(heatRows, 0, sizeof(heatRows));
    sweeps = 0;
    hotBucket = 0;
    hotLevel = 0;
    frameTick = 0;
}

void rfHeatmapExit() {
    activeRadio->stopListening();
    memset(heatRows, 0, sizeof(heatRows));
}

void rfHeatmapLoop() {
    if (Input.pressed(BTN_ID_OK)) {
        memset(heatRows, 0, sizeof(heatRows));
        sweeps = 0;
        Input.consume(BTN_ID_OK);
    }

    uint8_t row[BUCKET_COUNT];
    scanHeatRow(row);
    pushHeatRow(row);
    frameTick++;

    u8g2.clearBuffer();
    drawHeatmap();
    u8g2.sendBuffer(); oledMirrorSync();
}
