
import re

with open("src/gui_helper.cpp", "r") as f:
    text = f.read()

# Replace signatures and contents
def repl_wifi(m):
    return """void drawWifiCategoryIcon(int x, int y, uint8_t frame, float scale) {
    uint8_t step = (frame / 6) % 4;
    u8g2.drawDisc(x + 16*scale, y + 22*scale, max(1.0f, 2*scale));
    if (step >= 1) u8g2.drawCircle(x + 16*scale, y + 22*scale, 6*scale,  U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x + 16*scale, y + 22*scale, 10*scale, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    if (step >= 3) u8g2.drawCircle(x + 16*scale, y + 22*scale, 14*scale, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
}"""

def repl_rf(m):
    return """void drawRfCategoryIcon(int x, int y, uint8_t frame, float scale) {
    static const int8_t sx[] = {0, 8, 12, 7, 0, -7, -12, -8};
    static const int8_t sy[] = {-12, -9, 0, 8, 12, 8, 0, -9};
    uint8_t pos  = (frame / 4) & 7;
    uint8_t tail = (pos - 1) & 7;
    u8g2.drawCircle(x + 16*scale, y + 16*scale, 14*scale);
    u8g2.drawDisc(x + 16*scale, y + 16*scale, max(1.0f, 2*scale));
    u8g2.drawPixel(x + 16*scale + (sx[tail]/2)*scale, y + 16*scale + (sy[tail]/2)*scale);
    u8g2.drawLine(x + 16*scale, y + 16*scale, x + 16*scale + sx[pos]*scale, y + 16*scale + sy[pos]*scale);
    u8g2.drawDisc(x + 16*scale + sx[pos]*scale, y + 16*scale + sy[pos]*scale, max(1.0f, 2*scale));
}"""

def repl_bt(m):
    return """void drawBluetoothCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawVLine(x + 16*scale, y + 6*scale, 20*scale);
    u8g2.drawLine(x + 16*scale, y + 26*scale, x + 24*scale, y + 18*scale);
    u8g2.drawLine(x + 24*scale, y + 18*scale, x + 8*scale, y + 6*scale);
    u8g2.drawLine(x + 16*scale, y + 6*scale, x + 24*scale, y + 14*scale);
    u8g2.drawLine(x + 24*scale, y + 14*scale, x + 8*scale, y + 26*scale);
    uint8_t step = (frame / 8) % 3;
    if (step >= 1) u8g2.drawCircle(x + 26*scale, y + 16*scale, 2*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
    if (step >= 2) u8g2.drawCircle(x + 26*scale, y + 16*scale, 4*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
}"""

def repl_warn(m):
    return """void drawWarningCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawCircle(x + 16*scale, y + 16*scale, 11*scale);
    u8g2.drawLine(x + 8*scale, y + 24*scale, x + 24*scale, y + 8*scale);
    uint8_t pulse = (frame / 5) % 4;
    if (pulse == 1) u8g2.drawCircle(x + 16*scale, y + 16*scale, 13*scale);
    if (pulse == 2) u8g2.drawCircle(x + 16*scale, y + 16*scale, 15*scale);
}"""

def repl_sys(m):
    return """void drawSystemCategoryIcon(int x, int y, uint8_t frame, float scale) {
    static const int8_t ox[] = {11, 8, 0, -8, -11, -8, 0, 8};
    static const int8_t oy[] = {0, 8, 11, 8, 0, -8, -11, -8};
    uint8_t pos = (frame / 6) & 7;
    u8g2.drawCircle(x + 16*scale, y + 16*scale, 11*scale);
    u8g2.drawDisc(x + 16*scale, y + 16*scale, max(1.0f, 4*scale));
    u8g2.drawHLine(x + 2*scale, y + 15*scale, 7*scale);
    u8g2.drawHLine(x + 23*scale, y + 15*scale, 7*scale);
    u8g2.drawVLine(x + 15*scale, y + 2*scale, 7*scale);
    u8g2.drawVLine(x + 15*scale, y + 23*scale, 7*scale);
    u8g2.drawDisc(x + 16*scale + ox[pos]*scale, y + 16*scale + oy[pos]*scale, max(1.0f, 1*scale));
}"""

def repl_games(m):
    return """void drawGamesCategoryIcon(int x, int y, uint8_t frame, float scale) {
    uint8_t blink = (frame / 8) & 1;
    u8g2.drawRFrame(x + 3*scale, y + 11*scale, 26*scale, 14*scale, max(1.0f, 4*scale));
    u8g2.drawDisc(x + 10*scale, y + 18*scale, max(1.0f, 2*scale));
    u8g2.drawHLine(x + 7*scale, y + 18*scale, 6*scale);
    u8g2.drawVLine(x + 10*scale, y + 15*scale, 6*scale);
    u8g2.drawDisc(x + 21*scale, y + 16*scale, max(1.0f, (1 + blink)*scale));
    u8g2.drawDisc(x + 25*scale, y + 20*scale, max(1.0f, 1*scale));
}"""

def repl_ir(m):
    return """void drawIRCategoryIcon(int x, int y, uint8_t frame, float scale) {
    u8g2.drawBox(x + 8*scale, y + 14*scale, 16*scale, 12*scale);
    u8g2.drawDisc(x + 16*scale, y + 14*scale, max(1.0f, 4*scale));
    uint8_t wave = (frame / 4) % 3;
    if (wave >= 0) u8g2.drawCircle(x + 16*scale, y + 14*scale, 8*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 1) u8g2.drawCircle(x + 16*scale, y + 14*scale, 12*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    if (wave >= 2) u8g2.drawCircle(x + 16*scale, y + 14*scale, 16*scale, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
}"""

text = re.sub(r"void drawWifiCategoryIcon.*?}", repl_wifi(None), text, flags=re.DOTALL)
text = re.sub(r"void drawRfCategoryIcon.*?}", repl_rf(None), text, flags=re.DOTALL)
text = re.sub(r"void drawBluetoothCategoryIcon.*?}", repl_bt(None), text, flags=re.DOTALL)
text = re.sub(r"void drawWarningCategoryIcon.*?}", repl_warn(None), text, flags=re.DOTALL)
text = re.sub(r"void drawSystemCategoryIcon.*?}", repl_sys(None), text, flags=re.DOTALL)
text = re.sub(r"void drawGamesCategoryIcon.*?}", repl_games(None), text, flags=re.DOTALL)
text = re.sub(r"void drawIRCategoryIcon.*?}", repl_ir(None), text, flags=re.DOTALL)

# update drawCategoryIcon
text = re.sub(r"static void drawCategoryIcon\(uint8_t icon, int x, int y, bool animate = true\)", "static void drawCategoryIcon(uint8_t icon, int x, int y, bool animate = true, float scale = 1.0f)", text)
text = re.sub(r"drawWifiCategoryIcon\(x, y, frame\);", "drawWifiCategoryIcon(x, y, frame, scale);", text)
text = re.sub(r"drawRfCategoryIcon\(x, y, frame\);", "drawRfCategoryIcon(x, y, frame, scale);", text)
text = re.sub(r"drawBluetoothCategoryIcon\(x, y, frame\);", "drawBluetoothCategoryIcon(x, y, frame, scale);", text)
text = re.sub(r"drawWarningCategoryIcon\(x, y, frame\);", "drawWarningCategoryIcon(x, y, frame, scale);", text)
text = re.sub(r"drawGamesCategoryIcon\(x, y, frame\);", "drawGamesCategoryIcon(x, y, frame, scale);", text)
text = re.sub(r"drawIRCategoryIcon\(x, y, frame\);", "drawIRCategoryIcon(x, y, frame, scale);", text)
text = re.sub(r"drawSystemCategoryIcon\(x, y, frame\);", "drawSystemCategoryIcon(x, y, frame, scale);", text)

with open("src/gui_helper.cpp", "w") as f:
    f.write(text)


