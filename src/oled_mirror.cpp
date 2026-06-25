#include "oled_mirror.h"
#include <U8g2lib.h>
#include <base64.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool isTotalAttacking;
extern bool forceBleMode;

static uint8_t lastFrame[1024];
static unsigned long lastMirrorMs = 0;
const unsigned long MIRROR_INTERVAL_MS = 100; // 10 FPS

void oledMirrorSetup() {
    memset(lastFrame, 0, sizeof(lastFrame));
}

void oledMirrorSync() {
    // Pause mirror during heavy RF operations
    if (isTotalAttacking || forceBleMode) return;

    unsigned long now = millis();
    if (now - lastMirrorMs < MIRROR_INTERVAL_MS) return;
    
    uint8_t* currentFrame = u8g2.getBufferPtr();
    if (!currentFrame) return;

    // Only transmit when framebuffer changes
    if (memcmp(lastFrame, currentFrame, 1024) != 0) {
        memcpy(lastFrame, currentFrame, 1024);
        
        // Encode base64
        String b64 = base64::encode(currentFrame, 1024);
        
        // Send chunked to avoid serial flooding
        Serial.println("[OLED_FRAME_BEGIN]");
        Serial.println("WIDTH:128");
        Serial.println("HEIGHT:64");
        int len = b64.length();
        int offset = 0;
        int chunk_size = 128;
        
        while (offset < len) {
            String chunk = b64.substring(offset, offset + chunk_size);
            if (offset == 0) {
                Serial.print("DATA:");
            }
            Serial.println(chunk);
            offset += chunk_size;
            // Removed yield() to ensure this finishes as fast as possible, 
            // but keep it if WDT barks. Let's keep it just in case.
            yield(); 
        }
        Serial.println("[OLED_FRAME_END]");
        
        lastMirrorMs = now;
    }
}
