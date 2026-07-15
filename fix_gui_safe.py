
import os

with open("src/gui_helper.cpp", "r", encoding="utf-8") as f:
    text = f.read().replace("\r", "")

def safe_replace(old, new):
    global text
    if old in text:
        text = text.replace(old, new)
    else:
        print(f"Failed to find: {old[:50]}...")

wifi_old = """void drawWifiCategoryIcon(int x, int y, uint8_t frame) {
    uint8_t step = (frame / 6) % 4;
    u8g2.drawDisc(x+16, y+22, 2);
    if (step >= 1) u8g2.drawCircle(x+16, y+22, 6,  U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x+16, y+22, 10, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 3) u8g2.drawCircle(x+16, y+22, 14, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
}"""
wifi_new = """void drawWifiCategoryIcon(int x, int y, uint8_t frame, float scale) {
    uint8_t step = (frame / 6) % 4;
    u8g2.drawDisc(x+16*scale, y+22*scale, max(1.0f, 2*scale));
    if (step >= 1) u8g2.drawCircle(x+16*scale, y+22*scale, 6*scale,  U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x+16*scale, y+22*scale, 10*scale, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 3) u8g2.drawCircle(x+16*scale, y+22*scale, 14*scale, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
}"""
safe_replace(wifi_old, wifi_new)

rf_old = """void drawRfCategoryIcon(int x, int y, uint8_t frame) {
    static const int8_t sx[] = {0, 8, 12, 7, 0, -7, -12, -8};
    static const int8_t sy[] = {-12, -9, 0, 8, 12, 8, 0, -9};
    uint8_t pos  = (frame / 4) & 7;
    uint8_t tail = (pos - 1) & 7;

    u8g2.drawCircle(x+16, y+16, 14);
    u8g2.drawDisc(x+16, y+16, 2);
    u8g2.drawPixel(x+16+sx[tail]/2, y+16+sy[tail]/2);
    u8g2.drawLine(x+16, y+16, x+16+sx[pos], y+16+sy[pos]);
    u8g2.drawDisc(x+16+sx[pos], y+16+sy[pos], 2);
}"""
rf_new = """void drawRfCategoryIcon(int x, int y, uint8_t frame, float scale) {
    static const int8_t sx[] = {0, 8, 12, 7, 0, -7, -12, -8};
    static const int8_t sy[] = {-12, -9, 0, 8, 12, 8, 0, -9};
    uint8_t pos  = (frame / 4) & 7;
    uint8_t tail = (pos - 1) & 7;
    u8g2.drawCircle(x+16*scale, y+16*scale, 14*scale);
    u8g2.drawDisc(x+16*scale, y+16*scale, max(1.0f, 2*scale));
    u8g2.drawPixel(x+16*scale+(sx[tail]/2)*scale, y+16*scale+(sy[tail]/2)*scale);
    u8g2.drawLine(x+16*scale, y+16*scale, x+16*scale+sx[pos]*scale, y+16*scale+sy[pos]*scale);
    u8g2.drawDisc(x+16*scale+sx[pos]*scale, y+16*scale+sy[pos]*scale, max(1.0f, 2*scale));
}"""
safe_replace(rf_old, rf_new)

bt_old = """void drawBluetoothCategoryIcon(int x, int y, uint8_t frame) {
    drawBluetoothIcon(x, y);
    uint8_t step = (frame / 8) % 3;
    if (step >= 1) u8g2.drawCircle(x+26, y+16, 2, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x+26, y+16, 4, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
}"""
bt_new = """void drawBluetoothCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawVLine(x+16*scale, y+6*scale, 20*scale);
    u8g2.drawLine(x+16*scale, y+26*scale, x+24*scale, y+18*scale);
    u8g2.drawLine(x+24*scale, y+18*scale, x+8*scale, y+6*scale);
    u8g2.drawLine(x+16*scale, y+6*scale, x+24*scale, y+14*scale);
    u8g2.drawLine(x+24*scale, y+14*scale, x+8*scale, y+26*scale);
    uint8_t step = (frame / 8) % 3;
    if (step >= 1) u8g2.drawCircle(x+26*scale, y+16*scale, 2*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x+26*scale, y+16*scale, 4*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
}"""
safe_replace(bt_old, bt_new)

warn_old = """void drawWarningCategoryIcon(int x, int y, uint8_t frame) {
    // simbolo de prohibido base (siempre visible)
    u8g2.drawCircle(x+16, y+16, 11);
    u8g2.drawLine(x+8, y+24, x+24, y+8);

    // onda de alerta expansiva: aparece, crece y descansa
    uint8_t pulse = (frame / 5) % 4;
    if (pulse == 1) u8g2.drawCircle(x+16, y+16, 13);
    if (pulse == 2) u8g2.drawCircle(x+16, y+16, 15);
}"""
warn_new = """void drawWarningCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawCircle(x+16*scale, y+16*scale, 11*scale);
    u8g2.drawLine(x+8*scale, y+24*scale, x+24*scale, y+8*scale);
    uint8_t pulse = (frame / 5) % 4;
    if (pulse == 1) u8g2.drawCircle(x+16*scale, y+16*scale, 13*scale);
    if (pulse == 2) u8g2.drawCircle(x+16*scale, y+16*scale, 15*scale);
}"""
safe_replace(warn_old, warn_new)

sys_old = """void drawSystemCategoryIcon(int x, int y, uint8_t frame) {
    static const int8_t ox[] = {11, 8, 0, -8, -11, -8, 0, 8};
    static const int8_t oy[] = {0, 8, 11, 8, 0, -8, -11, -8};
    uint8_t pos = (frame / 6) & 7;

    u8g2.drawCircle(x+16, y+16, 11);
    u8g2.drawDisc(x+16, y+16, 4);
    u8g2.drawHLine(x+2, y+15, 7);
    u8g2.drawHLine(x+23, y+15, 7);
    u8g2.drawVLine(x+15, y+2, 7);
    u8g2.drawVLine(x+15, y+23, 7);
    u8g2.drawDisc(x+16+ox[pos], y+16+oy[pos], 1);
}"""
sys_new = """void drawSystemCategoryIcon(int x, int y, uint8_t frame, float scale) {
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
}"""
safe_replace(sys_old, sys_new)

games_old = """void drawGamesCategoryIcon(int x, int y, uint8_t frame) {
    uint8_t blink = (frame / 8) & 1;
    u8g2.drawRFrame(x+3, y+11, 26, 14, 4);
    u8g2.drawDisc(x+10, y+18, 2);
    u8g2.drawHLine(x+7, y+18, 6);
    u8g2.drawVLine(x+10, y+15, 6);
    u8g2.drawDisc(x+21, y+16, 1 + blink);
    u8g2.drawDisc(x+25, y+20, 1);
}"""
games_new = """void drawGamesCategoryIcon(int x, int y, uint8_t frame, float scale) {
    uint8_t blink = (frame / 8) & 1;
    u8g2.drawRFrame(x+3*scale, y+11*scale, 26*scale, 14*scale, max(1.0f, 4*scale));
    u8g2.drawDisc(x+10*scale, y+18*scale, max(1.0f, 2*scale));
    u8g2.drawHLine(x+7*scale, y+18*scale, 6*scale);
    u8g2.drawVLine(x+10*scale, y+15*scale, 6*scale);
    u8g2.drawDisc(x+21*scale, y+16*scale, max(1.0f, (1 + blink)*scale));
    u8g2.drawDisc(x+25*scale, y+20*scale, max(1.0f, 1*scale));
}"""
safe_replace(games_old, games_new)

ir_old = """void drawIRCategoryIcon(int x, int y, uint8_t frame) {
    u8g2.drawBox(x+8, y+14, 16, 12);
    u8g2.drawDisc(x+16, y+14, 4);
    uint8_t wave = (frame / 4) % 3;
    if (wave >= 0) u8g2.drawCircle(x+16, y+14, 8, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 1) u8g2.drawCircle(x+16, y+14, 12, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 2) u8g2.drawCircle(x+16, y+14, 16, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
}"""
ir_new = """void drawIRCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawBox(x+8*scale, y+14*scale, 16*scale, 12*scale);
    u8g2.drawDisc(x+16*scale, y+14*scale, max(1.0f, 4*scale));
    uint8_t wave = (frame / 4) % 3;
    if (wave >= 0) u8g2.drawCircle(x+16*scale, y+14*scale, 8*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 1) u8g2.drawCircle(x+16*scale, y+14*scale, 12*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 2) u8g2.drawCircle(x+16*scale, y+14*scale, 16*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
}"""
safe_replace(ir_old, ir_new)

cat_old = """static void drawCategoryIcon(uint8_t icon, int x, int y) {
    uint8_t frame = (millis() / 80) & 0xFF;
    switch (icon) {
        case MENU_ICON_WIFI:
            drawWifiCategoryIcon(x, y, frame);
            break;
        case MENU_ICON_RF:
            drawRfCategoryIcon(x, y, frame);
            break;
        case MENU_ICON_BLUETOOTH:
            drawBluetoothCategoryIcon(x, y, frame);
            break;
        case MENU_ICON_WARNING:
            drawWarningCategoryIcon(x, y, frame);
            break;
        case MENU_ICON_GAMES:
            drawGamesCategoryIcon(x, y, frame);
            break;
        case MENU_ICON_IR:
            drawIRCategoryIcon(x, y, frame);
            break;
        case MENU_ICON_SYSTEM:
            drawSystemCategoryIcon(x, y, frame);
            break;
        default:
            drawInfo(x, y);
            break;
    }
}"""
cat_new = """static void drawCategoryIcon(uint8_t icon, int x, int y, bool animate = true, float scale = 1.0f) {
    uint8_t frame = animate ? ((millis() / 80) & 0xFF) : 0;
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
}"""
safe_replace(cat_old, cat_new)

# Now replace the screen drawing entirely to have 3 rectangular cards
screen_old = """static void drawCategoryScreen() {
    uint8_t total = menuCategoryCount();
    const MenuCategory& cat = menuCategoryAt(category_index);

    u8g2.clearBuffer();
    char status[8];
    snprintf(status, sizeof(status), "%02d/%02d", category_index + 1, total);
    UiTheme::drawHeader(u8g2, "TetraX v4.0", status);

    drawCategoryIcon(cat.icon, 48, 18);
    drawCategoryRail(total, category_index);

    const char* label = cat.name;
    u8g2.setFont(u8g2_font_6x10_tr);
    int labelWidth = u8g2.getStrWidth(label);
    int labelX = (128 - labelWidth) / 2;
    u8g2.drawBox(max(2, labelX - 4), 53, min(124, labelWidth + 8), 10);
    u8g2.setDrawColor(0);
    u8g2.drawStr(labelX, 61, label);
    u8g2.setDrawColor(1);

    u8g2.drawRFrame(0, 0, 128, 64, 5);
    u8g2.sendBuffer(); oledMirrorSync();
}"""
screen_new = """static void drawCategoryScreen() {
    u8g2.clearBuffer();
    
    int total = menuCategoryCount();
    int bounceOffset = (millis() / 200) % 2;
    const MenuCategory& cat = menuCategoryAt(category_index);
    
    // Draw 3 cards
    for (int i = 0; i < 3; i++) {
        int idx = category_index + i - 1;
        if (idx < 0) idx = total - 1;
        if (idx >= total) idx = 0;
        
        const MenuCategory& c = menuCategoryAt(idx);
        int bx, by, bw, bh;
        float scale;
        int iconOffset;
        
        if (i == 1) { // Center
            bw = 36; bh = 44;
            bx = 46; by = 10 + bounceOffset;
            scale = 0.9f;
            iconOffset = 2; // Offset for centered icon
        } else { // Sides
            bw = 26; bh = 34;
            bx = (i == 0) ? 14 : 88; 
            by = 15;
            scale = 0.6f;
            iconOffset = 3;
        }
        
        u8g2.setDrawColor(1);
        if (i == 1) {
            u8g2.drawRFrame(bx, by, bw, bh, 4);
            drawCategoryIcon(c.icon, bx + iconOffset, by + iconOffset + 2, true, scale);
        } else {
            u8g2.drawRFrame(bx, by, bw, bh, 3);
            drawCategoryIcon(c.icon, bx + iconOffset, by + iconOffset + 2, false, scale);
        }
    }
    
    // Draw label
    const char* label = cat.name;
    u8g2.setFont(u8g2_font_6x10_tr);
    int labelWidth = u8g2.getStrWidth(label);
    int labelX = (128 - labelWidth) / 2;
    
    u8g2.drawBox(max(2, labelX - 4), 54, min(124, labelWidth + 8), 10);
    u8g2.setDrawColor(0);
    u8g2.drawStr(labelX, 62, label);
    u8g2.setDrawColor(1);

    u8g2.sendBuffer(); oledMirrorSync();
}"""
safe_replace(screen_old, screen_new)

with open("src/gui_helper.cpp", "w", encoding="utf-8") as f:
    f.write(text)


