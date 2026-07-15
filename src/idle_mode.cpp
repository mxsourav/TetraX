#include "idle_mode.h"
#include "animations.h"
#include "oled_mirror.h"
#include "input_manager.h"
#include "app_config.h"
#include "buzzer_manager.h"
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

#ifndef totalAutoImages
// Based on Hizmos-main implementation
extern const uint8_t* autoImages[];
const int totalAutoImages = 406;
#endif

void idleModeLoop() {
    int autoImageIndex = 0;
    unsigned long lastImageChangeTime = 0;
    
    // Set InputManager to idle mode so all keys except OK are completely ignored
    Input.setIdleMode(true);
    
    // Clear any pending button presses
    Input.update();
    
    // Force first draw
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.drawXBMP(0, 0, 128, 64, autoImages[autoImageIndex]);
    u8g2.sendBuffer();
    oledMirrorSync();

    while (true) {
        Input.update();
        
        // ONLY the OK button exits idle mode
        if (Input.pressed(BTN_ID_OK)) {
            BuzzerManager::beep(5);
            Input.setIdleMode(false); // Restore normal inputs
            return;
        }

        if (millis() - lastImageChangeTime > 66) { // ~15 FPS
            autoImageIndex = (autoImageIndex + 1) % totalAutoImages;
            lastImageChangeTime = millis();
            
            u8g2.clearBuffer();
            u8g2.setDrawColor(1);
            
            // Draw dolphin frame (128x64 XBM)
            u8g2.drawXBMP(0, 0, 128, 64, autoImages[autoImageIndex]);
            
            u8g2.sendBuffer();
            oledMirrorSync();
        }
        
        yield();
    }
}
