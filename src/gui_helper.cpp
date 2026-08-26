#include "gui_helper.h"
#include "oled_mirror.h"
#include "ui_theme.h"
#include "menu_catalog.h"
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern int menu_index;
extern int category_index;
extern int category_app_index;
extern bool browsingCategoryApps;
extern float currentPos;
extern int TOTAL_OPTIONS;
extern const char* menu_labels[];
extern bool attack_confirmed;

static void drawPositionRail() {
    int railX = 122;
    int railY = 18;
    int railH = 32;
    int markerH = max(3, railH / TOTAL_OPTIONS);
    int markerY = railY + ((railH - markerH) * menu_index) / max(1, TOTAL_OPTIONS - 1);

    u8g2.drawVLine(railX, railY, railH);
    u8g2.drawBox(railX - 1, markerY, 3, markerH);
}

static void drawCategoryRail(uint8_t total, uint8_t selected) {
    int railX = 122;
    int railY = 18;
    int railH = 32;
    int markerH = max(5, railH / max(1, (int)total));
    int markerY = railY + ((railH - markerH) * selected) / max(1, (int)total - 1);

    u8g2.drawVLine(railX, railY, railH);
    u8g2.drawBox(railX - 1, markerY, 3, markerH);
}

static void drawSelectedLabel() {
    const char* label = menu_labels[menu_index];

    u8g2.setFont(u8g2_font_6x10_tr);
    int labelWidth = u8g2.getStrWidth(label);
    int labelX = (128 - labelWidth) / 2;

    u8g2.setDrawColor(1);
    u8g2.drawBox(max(2, labelX - 4), 53, min(124, labelWidth + 8), 10);
    u8g2.setDrawColor(0);
    u8g2.drawStr(labelX, 61, label);
    u8g2.setDrawColor(1);
}

// --- COLECCIÓN DE ICONOS ÚNICOS ---
void drawBolt(int x, int y) { u8g2.drawLine(x+22, y+2, x+10, y+16); u8g2.drawLine(x+10, y+16, x+24, y+16); u8g2.drawLine(x+24, y+16, x+12, y+30); }
void drawFlame(int x, int y) { u8g2.drawTriangle(x+16, y+2, x+8, y+20, x+24, y+20); u8g2.drawDisc(x+16, y+22, 8); u8g2.setDrawColor(0); u8g2.drawDisc(x+16, y+24, 4); u8g2.setDrawColor(1); }
void drawBan(int x, int y) { u8g2.drawCircle(x+16, y+16, 13); u8g2.drawLine(x+7, y+25, x+25, y+7); }
void drawBTJam(int x, int y) { u8g2.drawLine(x+12, y+6, x+12, y+26); u8g2.drawLine(x+12, y+6, x+20, y+11); u8g2.drawLine(x+20, y+11, x+12, y+16); u8g2.drawLine(x+12, y+16, x+20, y+21); u8g2.drawLine(x+20, y+21, x+12, y+26); u8g2.drawLine(x+6, y+6, x+26, y+26); }
void drawRadar(int x, int y) { u8g2.drawCircle(x+16, y+16, 14); u8g2.drawCircle(x+16, y+16, 7); u8g2.drawLine(x+16, y+2, x+16, y+30); u8g2.drawLine(x+2, y+16, x+30, y+16); u8g2.drawLine(x+16, y+16, x+28, y+5); }
void drawSearch(int x, int y) { u8g2.drawCircle(x+14, y+12, 9); u8g2.drawLine(x+20, y+19, x+28, y+28); }
void drawMegaphone(int x, int y) { u8g2.drawTriangle(x+8, y+16, x+24, y+6, x+24, y+26); u8g2.drawBox(x+4, y+14, 6, 6); }
void drawLock(int x, int y) { u8g2.drawFrame(x+8, y+14, 16, 13); u8g2.drawCircle(x+16, y+14, 7, U8G2_DRAW_UPPER_RIGHT|U8G2_DRAW_UPPER_LEFT); u8g2.drawDisc(x+16, y+21, 2); }
void drawAlien(int x, int y) { u8g2.drawBox(x+6, y+10, 20, 10); u8g2.drawBox(x+4, y+12, 24, 6); u8g2.drawBox(x+8, y+22, 4, 4); u8g2.drawBox(x+20, y+22, 4, 4); u8g2.setDrawColor(0); u8g2.drawBox(x+9, y+13, 4, 4); u8g2.drawBox(x+19, y+13, 4, 4); u8g2.setDrawColor(1); }
void drawFolder(int x, int y) { u8g2.drawFrame(x+4, y+8, 24, 18); u8g2.drawBox(x+4, y+4, 10, 4); }
void drawInfo(int x, int y) { u8g2.drawCircle(x+16, y+16, 14); u8g2.drawBox(x+15, y+12, 2, 10); u8g2.drawDisc(x+16, y+8, 2); }
void drawSnakeIcon(int x, int y) { u8g2.drawFrame(x+6, y+6, 20, 20); u8g2.drawBox(x+10, y+10, 4, 4); u8g2.drawBox(x+14, y+10, 4, 4); u8g2.drawBox(x+14, y+14, 4, 4); u8g2.drawDisc(x+22, y+22, 2); }
void drawMonitorIcon(int x, int y) { u8g2.drawFrame(x+4, y+6, 24, 20); u8g2.drawLine(x+6, y+20, x+12, y+10); u8g2.drawLine(x+12, y+10, x+18, y+24); u8g2.drawLine(x+18, y+24, x+24, y+14); }
void drawScanIcon(int x, int y) { u8g2.drawCircle(x+14, y+12, 7); u8g2.drawLine(x+18, y+18, x+24, y+24); u8g2.drawHLine(x+4, y+28, 24); }
void drawControlIcon(int x, int y) { u8g2.drawFrame(x+6, y+10, 20, 12); u8g2.drawBox(x+14, y+22, 4, 4); u8g2.drawCircle(x+16, y+16, 3); }
void drawRemoteIcon(int x, int y) { u8g2.drawFrame(x+10, y+4, 12, 24); u8g2.drawBox(x+13, y+7, 6, 4); u8g2.drawBox(x+13, y+13, 2, 2); u8g2.drawBox(x+17, y+13, 2, 2); u8g2.drawBox(x+13, y+17, 6, 2); u8g2.drawDisc(x+16, y+23, 2); }
void drawBluetoothIcon(int x, int y) { u8g2.drawLine(x+15, y+4, x+15, y+28); u8g2.drawLine(x+15, y+4, x+24, y+11); u8g2.drawLine(x+24, y+11, x+15, y+17); u8g2.drawLine(x+15, y+17, x+24, y+23); u8g2.drawLine(x+24, y+23, x+15, y+28); u8g2.drawLine(x+7, y+10, x+23, y+22); u8g2.drawLine(x+7, y+22, x+23, y+10); }
void drawBtAnalyzerIcon(int x, int y) { u8g2.drawLine(x+7, y+4, x+7, y+28); u8g2.drawLine(x+7, y+4, x+15, y+10); u8g2.drawLine(x+15, y+10, x+7, y+16); u8g2.drawLine(x+7, y+16, x+15, y+22); u8g2.drawLine(x+15, y+22, x+7, y+28); u8g2.drawFrame(x+19, y+8, 10, 18); u8g2.drawLine(x+20, y+23, x+22, y+19); u8g2.drawLine(x+22, y+19, x+24, y+22); u8g2.drawLine(x+24, y+22, x+28, y+12); }
void drawBtSpectrumIcon(int x, int y) { u8g2.drawLine(x+5, y+5, x+5, y+28); u8g2.drawLine(x+5, y+5, x+13, y+11); u8g2.drawLine(x+13, y+11, x+5, y+17); u8g2.drawLine(x+5, y+17, x+13, y+23); u8g2.drawLine(x+13, y+23, x+5, y+28); for (int i = 0; i < 6; i++) { int h = 4 + ((i * 5) % 15); u8g2.drawBox(x+16+i*2, y+27-h, 1, h); } u8g2.drawHLine(x+15, y+28, 14); }
void drawRfHeatmapIcon(int x, int y) { u8g2.drawFrame(x+4, y+5, 24, 22); for (int row = 0; row < 4; row++) { for (int col = 0; col < 6; col++) { if (((row * 3 + col * 2) % 5) < 3) u8g2.drawBox(x+7+col*3, y+8+row*4, 2, 2); else u8g2.drawPixel(x+8+col*3, y+9+row*4); } } }
void drawChannelAdvisorIcon(int x, int y) { u8g2.drawFrame(x+5, y+7, 22, 16); u8g2.drawLine(x+8, y+20, x+12, y+14); u8g2.drawLine(x+12, y+14, x+17, y+17); u8g2.drawLine(x+17, y+17, x+24, y+10); u8g2.drawDisc(x+24, y+10, 2); u8g2.drawLine(x+11, y+26, x+21, y+26); u8g2.drawLine(x+16, y+23, x+16, y+28); }
void drawNrfLinkIcon(int x, int y) { u8g2.drawFrame(x+4, y+9, 9, 13); u8g2.drawFrame(x+20, y+9, 9, 13); u8g2.drawLine(x+8, y+9, x+8, y+4); u8g2.drawLine(x+24, y+9, x+24, y+4); u8g2.drawDisc(x+8, y+24, 1); u8g2.drawDisc(x+24, y+24, 1); u8g2.drawCircle(x+16, y+15, 3); u8g2.drawCircle(x+16, y+15, 7); u8g2.drawLine(x+13, y+15, x+8, y+15); u8g2.drawLine(x+19, y+15, x+24, y+15); }
void drawNrfChatIcon(int x, int y) { u8g2.drawRFrame(x+4, y+7, 24, 15, 3); u8g2.drawTriangle(x+10, y+22, x+15, y+22, x+10, y+27); u8g2.drawHLine(x+8, y+12, 16); u8g2.drawHLine(x+8, y+16, 11); u8g2.drawCircle(x+24, y+25, 3); u8g2.drawCircle(x+24, y+25, 6, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT); }
void drawCoexIcon(int x, int y) { u8g2.drawFrame(x+3, y+7, 26, 18); u8g2.drawHLine(x+6, y+13, 20); u8g2.drawHLine(x+6, y+19, 20); u8g2.drawCircle(x+10, y+13, 2); u8g2.drawCircle(x+18, y+19, 2); u8g2.drawVLine(x+9, y+5, 23); u8g2.drawVLine(x+21, y+5, 23); u8g2.drawLine(x+5, y+27, x+27, y+5); }
void drawDualNrfScopeIcon(int x, int y) { u8g2.drawFrame(x+3, y+6, 26, 21); u8g2.drawHLine(x+5, y+16, 22); u8g2.drawLine(x+5, y+12, x+9, y+9); u8g2.drawLine(x+9, y+9, x+14, y+14); u8g2.drawLine(x+14, y+14, x+20, y+8); u8g2.drawLine(x+20, y+8, x+27, y+13); u8g2.drawLine(x+5, y+23, x+10, y+18); u8g2.drawLine(x+10, y+18, x+16, y+24); u8g2.drawLine(x+16, y+24, x+22, y+19); u8g2.drawLine(x+22, y+19, x+27, y+22); }
void drawSystemIcon(int x, int y) { u8g2.drawCircle(x+16, y+16, 11); u8g2.drawDisc(x+16, y+16, 4); u8g2.drawHLine(x+2, y+15, 7); u8g2.drawHLine(x+23, y+15, 7); u8g2.drawVLine(x+15, y+2, 7); u8g2.drawVLine(x+15, y+23, 7); }
void drawSkullIcon(int x, int y) {
    u8g2.drawCircle(x+16, y+13, 11);
    u8g2.drawBox(x+8, y+18, 16, 8);
    u8g2.setDrawColor(0);
    u8g2.drawDisc(x+12, y+13, 3);
    u8g2.drawDisc(x+20, y+13, 3);
    u8g2.drawTriangle(x+16, y+16, x+13, y+21, x+19, y+21);
    u8g2.setDrawColor(1);
    u8g2.drawVLine(x+11, y+23, 5);
    u8g2.drawVLine(x+16, y+22, 6);
    u8g2.drawVLine(x+21, y+23, 5);
    u8g2.drawHLine(x+9, y+27, 14);
}

// --- ICONOS ANIMADOS DE CATEGORIA ---

// WiFi: ondas concentricas que se propagan en cascada (4 fases)
void drawWifiCategoryIcon(int x, int y, uint8_t frame, float scale) {
    uint8_t step = (frame / 6) % 4;
    u8g2.drawDisc(x+16*scale, y+22*scale, max(1.0f, 2*scale));
    if (step >= 1) u8g2.drawCircle(x+16*scale, y+22*scale, 6*scale,  U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x+16*scale, y+22*scale, 10*scale, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 3) u8g2.drawCircle(x+16*scale, y+22*scale, 14*scale, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
}

// RF: barrido radar con estela de la posicion anterior
void drawRfCategoryIcon(int x, int y, uint8_t frame, float scale) {
    static const int8_t sx[] = {0, 8, 12, 7, 0, -7, -12, -8};
    static const int8_t sy[] = {-12, -9, 0, 8, 12, 8, 0, -9};
    uint8_t pos  = (frame / 4) & 7;
    uint8_t tail = (pos - 1) & 7;
    u8g2.drawCircle(x+16*scale, y+16*scale, 14*scale);
    u8g2.drawDisc(x+16*scale, y+16*scale, max(1.0f, 2*scale));
    u8g2.drawPixel(x+16*scale+(sx[tail]/2)*scale, y+16*scale+(sy[tail]/2)*scale);
    u8g2.drawLine(x+16*scale, y+16*scale, x+16*scale+sx[pos]*scale, y+16*scale+sy[pos]*scale);
    u8g2.drawDisc(x+16*scale+sx[pos]*scale, y+16*scale+sy[pos]*scale, max(1.0f, 2*scale));
}

// Bluetooth: simbolo BT estatico + ondas de emparejamiento saliendo a la derecha
void drawBluetoothCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawVLine(x+16*scale, y+6*scale, 20*scale);
    u8g2.drawLine(x+16*scale, y+26*scale, x+24*scale, y+18*scale);
    u8g2.drawLine(x+24*scale, y+18*scale, x+8*scale, y+6*scale);
    u8g2.drawLine(x+16*scale, y+6*scale, x+24*scale, y+14*scale);
    u8g2.drawLine(x+24*scale, y+14*scale, x+8*scale, y+26*scale);
    uint8_t step = (frame / 8) % 3;
    if (step >= 1) u8g2.drawCircle(x+26*scale, y+16*scale, 2*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x+26*scale, y+16*scale, 4*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
}

// Warning / ILEGAL: simbolo de prohibido (circulo tachado) con onda de alerta expansiva
void drawWarningCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawCircle(x+16*scale, y+16*scale, 11*scale);
    u8g2.drawLine(x+8*scale, y+24*scale, x+24*scale, y+8*scale);
    uint8_t pulse = (frame / 5) % 4;
    if (pulse == 1) u8g2.drawCircle(x+16*scale, y+16*scale, 13*scale);
    if (pulse == 2) u8g2.drawCircle(x+16*scale, y+16*scale, 15*scale);
}

// System: engranaje con particula recorriendo el perimetro
void drawSystemCategoryIcon(int x, int y, uint8_t frame, float scale) {
    static const int8_t ox[] = {11, 8, 0, -8, -11, -8, 0, 8};
    static const int8_t oy[] = {0, 8, 11, 8, 0, -8, -11, -8};
    uint8_t pos = (frame / 6) & 7;
    u8g2.drawCircle(x+16*scale, y+16*scale, 11*scale);
    u8g2.drawDisc(x+16*scale, y+16*scale, max(1.0f, 4*scale));
    u8g2.drawHLine(x+2*scale, y+15*scale, 7*scale);
    u8g2.drawHLine(x+23*scale, y+15*scale, 7*scale);
    u8g2.drawVLine(x+15*scale, y+2*scale, 7*scale);
    u8g2.drawVLine(x+15*scale, y+23*scale, 7*scale);
    u8g2.drawDisc(x+16*scale+ox[pos]*scale, y+16*scale+oy[pos]*scale, max(1.0f, 1*scale));
}

void drawGamesCategoryIcon(int x, int y, uint8_t frame, float scale) {
    uint8_t blink = (frame / 8) & 1;
    u8g2.drawRFrame(x+3*scale, y+11*scale, 26*scale, 14*scale, max(1.0f, 4*scale));
    u8g2.drawDisc(x+10*scale, y+18*scale, max(1.0f, 2*scale));
    u8g2.drawHLine(x+7*scale, y+18*scale, 6*scale);
    u8g2.drawVLine(x+10*scale, y+15*scale, 6*scale);
    u8g2.drawDisc(x+21*scale, y+16*scale, max(1.0f, (1 + blink)*scale));
    u8g2.drawDisc(x+25*scale, y+20*scale, max(1.0f, 1*scale));
}

// --- NUEVOS DISEÑOS EXCLUSIVOS ---
void drawPortalIcon(int x, int y) { // EVIL PORTAL: Puerta con remolino
    u8g2.drawFrame(x+8, y+4, 16, 24);
    u8g2.drawCircle(x+16, y+14, 4);
    u8g2.drawCircle(x+16, y+14, 2);
    u8g2.drawHLine(x+4, y+28, 24);
}
void drawDashboardIcon(int x, int y) { // WEB DASHBOARD: Monitor de PC
    u8g2.drawFrame(x+4, y+6, 24, 16);
    u8g2.drawBox(x+12, y+22, 8, 2);
    u8g2.drawHLine(x+8, y+24, 16);
    u8g2.drawPixel(x+6, y+8); // Detalle pantalla
}
void drawIPScannerIcon(int x, int y) { // IP SCANNER: Lupa con "red"
    u8g2.drawCircle(x+12, y+12, 8);
    u8g2.drawLine(x+12, y+8, x+12, y+16);
    u8g2.drawLine(x+8, y+12, x+16, y+12);
    u8g2.drawLine(x+18, y+18, x+26, y+26);
}

void drawIRCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawBox(x+8*scale, y+14*scale, 16*scale, 12*scale);
    u8g2.drawDisc(x+16*scale, y+14*scale, max(1.0f, 4*scale));
    uint8_t wave = (frame / 4) % 3;
    if (wave >= 0) u8g2.drawCircle(x+16*scale, y+14*scale, 8*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 1) u8g2.drawCircle(x+16*scale, y+14*scale, 12*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 2) u8g2.drawCircle(x+16*scale, y+14*scale, 16*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
}

void drawIRRemoteAppIcon(int x, int y) {
    u8g2.drawFrame(x+10, y+4, 12, 24);
    u8g2.drawDisc(x+16, y+4, 2);
    u8g2.drawBox(x+12, y+8, 8, 4);
    u8g2.drawDisc(x+16, y+16, 2);
    u8g2.drawDisc(x+16, y+22, 2);
    u8g2.drawLine(x+16, y+2, x+12, y-2);
    u8g2.drawLine(x+16, y+2, x+20, y-2);
}

void drawTvRemoteIcon(int x, int y) {
    u8g2.drawFrame(x+4, y+6, 24, 16);
    u8g2.drawBox(x+12, y+22, 8, 2);
    u8g2.drawLine(x+20, y+8, x+28, y+2);
    u8g2.drawLine(x+20, y+12, x+30, y+6);
}

void drawAcRemoteIcon(int x, int y) {
    u8g2.drawLine(x+16, y+8, x+16, y+24);
    u8g2.drawLine(x+8, y+16, x+24, y+16);
    u8g2.drawLine(x+10, y+10, x+22, y+22);
    u8g2.drawLine(x+10, y+22, x+22, y+10);
}

void drawIrReceiverIcon(int x, int y) {
    u8g2.drawBox(x+12, y+16, 8, 8);
    u8g2.drawDisc(x+16, y+16, 4);
    u8g2.drawCircle(x+16, y+16, 8, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    u8g2.drawCircle(x+16, y+16, 12, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
}

void drawIrJammerIcon(int x, int y) {
    for(int i=0; i<4; i++) {
        u8g2.drawLine(x+4+(i*6), y+28, x+10+(i*6), y+4);
        u8g2.drawLine(x+4+(i*6), y+4, x+10+(i*6), y+28);
    }
}

void drawFromSdCardIcon(int x, int y) {
    u8g2.drawFrame(x+8, y+6, 16, 20);
    u8g2.drawLine(x+20, y+6, x+24, y+10);
    u8g2.drawHLine(x+10, y+10, 4);
    u8g2.drawHLine(x+10, y+14, 4);
    u8g2.drawLine(x+16, y+4, x+12, y+0);
    u8g2.drawLine(x+16, y+4, x+20, y+0);
}

void drawWifiRadarIcon(int x, int y) {
    u8g2.drawCircle(x+16, y+17, 4);
    u8g2.drawCircle(x+16, y+17, 10);
    u8g2.drawCircle(x+16, y+17, 15);
    u8g2.drawLine(x+16, y+17, x+27, y+7);
    u8g2.drawDisc(x+25, y+8, 2);
}

void drawChannelScanIcon(int x, int y) {
    u8g2.drawFrame(x+4, y+5, 24, 22);
    for (int i = 0; i < 5; i++) {
        int h = 5 + ((i * 4) % 15);
        u8g2.drawBox(x + 7 + i * 4, y + 25 - h, 3, h);
    }
    u8g2.drawHLine(x+6, y+28, 20);
}

void drawCentinelaIcon(int x, int y) {
    if (attack_confirmed && (millis() / 250) % 2 == 0) return;
    u8g2.drawFrame(x+8, y+4, 16, 18);
    u8g2.drawTriangle(x+8, y+22, x+24, y+22, x+16, y+30);
    u8g2.drawCircle(x+16, y+14, 4);
    u8g2.drawDisc(x+16, y+14, 1);
    u8g2.drawLine(x+4, y+10, x+8, y+10);
    u8g2.drawLine(x+24, y+10, x+28, y+10);
}

// --- DIBUJO DEL MENÚ PRINCIPAL ---
static void drawMenuIcon(int index, int x, int y) {
    switch(index) {
        case 0:  drawScanIcon(x, y); break;
        case 1:  drawWifiRadarIcon(x, y); break;
        case 2:  drawChannelScanIcon(x, y); break;
        case 3:  drawBolt(x, y); break;
        case 4:  drawSearch(x, y); break;
        case 5:  drawMonitorIcon(x, y); break;
        case 6:  drawCentinelaIcon(x, y); break;
        case 7:  drawBan(x, y); break;
        case 8:  drawRadar(x, y); break;
        case 9:  drawBTJam(x, y); break;
        case 10: drawMegaphone(x, y); break;
        case 11: drawFlame(x, y); break;
        case 12: drawAlien(x, y); break;
        case 13: drawPortalIcon(x, y); break;
        case 14: drawIPScannerIcon(x, y); break;
        case 15: drawControlIcon(x, y); break;
        case 16: drawDashboardIcon(x, y); break;
        case 17: drawIRRemoteAppIcon(x, y); break;
        case 18: drawFolder(x, y); break;
        case 19: drawSnakeIcon(x, y); break;
        case 20: drawInfo(x, y); break;
        case 21: drawBtAnalyzerIcon(x, y); break;
        case 22: drawBtSpectrumIcon(x, y); break;
        case 23: drawRfHeatmapIcon(x, y); break;
        case 24: drawChannelAdvisorIcon(x, y); break;
        case 25: drawNrfLinkIcon(x, y); break;
        case 26: drawNrfChatIcon(x, y); break;
        case 27: drawCoexIcon(x, y); break;
        case 28: drawDualNrfScopeIcon(x, y); break;
        case 29: drawSkullIcon(x, y); break;
        case 30: drawIrReceiverIcon(x, y); break;
        case 31: drawIrJammerIcon(x, y); break;
        case 32: drawFromSdCardIcon(x, y); break;
    }
}

static void drawCategoryIcon(uint8_t icon, int x, int y, bool animate = true, float scale = 1.0f) {
    uint8_t frame = animate ? ((millis() / 150) & 0xFF) : 0;
    switch (icon) {
        case MENU_ICON_WIFI:
            drawWifiCategoryIcon(x, y, frame, scale);
            break;
        case MENU_ICON_RF:
            drawRfCategoryIcon(x, y, frame, scale);
            break;
        case MENU_ICON_BLUETOOTH:
            drawBluetoothCategoryIcon(x, y, frame, scale);
            break;
        case MENU_ICON_WARNING:
            drawWarningCategoryIcon(x, y, frame, scale);
            break;
        case MENU_ICON_GAMES:
            drawGamesCategoryIcon(x, y, frame, scale);
            break;
        case MENU_ICON_IR:
            drawIRCategoryIcon(x, y, frame, scale);
            break;
        case MENU_ICON_SYSTEM:
            drawSystemCategoryIcon(x, y, frame, scale);
            break;
        default:
            drawInfo(x, y);
            break;
    }
}

static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static void getCardProps(float dt, int& x, int& y, int& w, int& h, float& scale) {
    float adt = abs(dt);
    float t;
    float cx, fW, fH, fY;
    if (adt <= 1.0f) {
        t = adt;
        cx = lerp(64, 96, t);
        fW = lerp(36, 24, t);
        fH = lerp(48, 38, t);
        fY = lerp(15, 20, t);
        scale = lerp(1.0f, 0.75f, t);
    } else if (adt <= 2.0f) {
        t = adt - 1.0f;
        cx = lerp(96, 119, t);
        fW = lerp(24, 18, t);
        fH = lerp(38, 28, t);
        fY = lerp(20, 25, t);
        scale = lerp(0.75f, 0.5f, t);
    } else if (adt <= 3.0f) {
        t = adt - 2.0f;
        cx = lerp(119, 138, t);
        fW = lerp(18, 0, t);
        fH = lerp(28, 0, t);
        fY = lerp(25, 32, t);
        scale = lerp(0.5f, 0.0f, t);
    } else {
        w = 0; h = 0; return;
    }
    
    if (dt < 0) {
        cx = 64 - (cx - 64);
    }
    
    w = (int)fW;
    h = (int)fH;
    x = (int)(cx - w / 2.0f);
    y = (int)fY;
}

static void drawCategoryScreen() {
    u8g2.clearBuffer();
    
    int total = menuCategoryCount();
    const MenuCategory& cat = menuCategoryAt(category_index);
    
    // Draw category name at top center inside a bubble
    const char* label = cat.name;
    u8g2.setFont(u8g2_font_helvB08_tr);
    int labelWidth = u8g2.getStrWidth(label);
    int labelX = (128 - labelWidth) / 2;
    
    u8g2.setDrawColor(1);
    u8g2.drawRBox(labelX - 4, 0, labelWidth + 8, 13, 3);
    u8g2.setDrawColor(0);
    u8g2.drawStr(labelX, 10, label);
    u8g2.setDrawColor(1);
    
    // Animation Logic
    static float anim_pos = -1;
    if (anim_pos < 0) anim_pos = category_index; // init
    
    static uint32_t last_time = millis();
    uint32_t now = millis();
    float dt_sec = (now - last_time) / 1000.0f;
    last_time = now;
    if (dt_sec > 0.1f) dt_sec = 0.1f; // Cap dt at 100ms
    
    float diff = category_index - anim_pos;
    if (diff > total / 2.0f) diff -= total;
    if (diff < -total / 2.0f) diff += total;
    
    if (abs(diff) > 0.01f) {
        float speed = 6.0f; // Smooth, natural card transition (~160ms)
        float step = speed * dt_sec;
        if (step > abs(diff)) step = abs(diff);
        anim_pos += (diff > 0) ? step : -step;
    } else {
        anim_pos = category_index;
    }
    
    if (anim_pos < 0) anim_pos += total;
    if (anim_pos >= total) anim_pos -= total;
    
    struct CardAnim {
        int idx;
        float dt;
        float adt;
    };
    CardAnim cards[10];
    int cCount = 0;
    
    for (int i = 0; i < total; i++) {
        float dt = i - anim_pos;
        while (dt > total / 2.0f) dt -= total;
        while (dt < -total / 2.0f) dt += total;
        
        if (abs(dt) < 3.0f) {
            cards[cCount].idx = i;
            cards[cCount].dt = dt;
            cards[cCount].adt = abs(dt);
            cCount++;
        }
    }
    
    // Sort descending by adt so center (adt=0) draws last and is on top
    for (int i = 0; i < cCount - 1; i++) {
        for (int j = 0; j < cCount - i - 1; j++) {
            if (cards[j].adt < cards[j+1].adt) {
                CardAnim temp = cards[j];
                cards[j] = cards[j+1];
                cards[j+1] = temp;
            }
        }
    }
    
    int bounceOffset = (millis() / 450) % 2;
    
    for (int i = 0; i < cCount; i++) {
        int x, y, w, h;
        float scale;
        getCardProps(cards[i].dt, x, y, w, h, scale);
        
        if (w <= 0 || h <= 0) continue;
        
        const MenuCategory& c = menuCategoryAt(cards[i].idx);
        bool isCenter = (cards[i].adt < 0.1f);
        if (isCenter) y += bounceOffset;
        
        // Mask out the background for this card so it overlays nicely
        u8g2.setDrawColor(0);
        u8g2.drawRBox(x, y, w, h, isCenter ? 3 : 2);
        
        u8g2.setDrawColor(1);
        if (isCenter) {
            u8g2.drawRFrame(x, y, w, h, 3);
            u8g2.drawRFrame(x+1, y+1, w-2, h-2, 2);
            int ax = x + w / 2;
            int ay = y + h - 4;
            u8g2.drawTriangle(ax - 3, ay - 2, ax + 3, ay - 2, ax, ay + 2);
            drawCategoryIcon(c.icon, x + 2, y + 4, true, 1.0f);
        } else {
            u8g2.drawRFrame(x, y, w, h, 2);
            int isz = (int)(32 * scale);
            int ix = x + (w - isz) / 2;
            int iy = y + (h - isz) / 2;
            drawCategoryIcon(c.icon, ix, iy, false, scale);
        }
    }

    u8g2.sendBuffer(); oledMirrorSync();
}

static void drawCategoryAppList() {
    const MenuCategory& cat = menuCategoryAt(category_index);
    int appIdx = menuCategoryAppIndex(category_index, category_app_index);

    u8g2.clearBuffer();
    char status[8];
    snprintf(status, sizeof(status), "%02d/%02d", category_app_index + 1, cat.count);
    UiTheme::drawHeader(u8g2, cat.name, status);

    drawMenuIcon(appIdx, 48, 18);
    drawCategoryRail(cat.count, category_app_index);

    extern int TOTAL_OPTIONS;
    extern const char* menu_labels[];
    
    const char* label = "[INVALID APP]";
    if (appIdx >= 0 && appIdx < TOTAL_OPTIONS && menu_labels[appIdx] != nullptr) {
        label = menu_labels[appIdx];
    }

    // Add Menu Navigation Diagnostics
    // Serial.printf("[SYSTEM_CURSOR] idx=%d\n", category_app_index);
    // Serial.printf("[SYSTEM_APP_ID] id=%d\n", appIdx);
    // Serial.printf("[SYSTEM_APP_NAME] name=%s\n", label);

    u8g2.setFont(u8g2_font_6x10_tr);
    int labelWidth = u8g2.getStrWidth(label);
    int labelX = (128 - labelWidth) / 2;
    u8g2.drawBox(max(2, labelX - 4), 53, min(124, labelWidth + 8), 10);
    u8g2.setDrawColor(0);
    u8g2.drawStr(labelX, 61, label);
    u8g2.setDrawColor(1);

    u8g2.drawRFrame(0, 0, 128, 64, 5);
    u8g2.sendBuffer(); oledMirrorSync();
}

void drawBruceMenu() {
    if (!browsingCategoryApps) {
        drawCategoryScreen();
        return;
    }

    drawCategoryAppList();
}
