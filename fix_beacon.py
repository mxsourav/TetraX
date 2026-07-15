
with open("src/beacon_spam.cpp", "r", encoding="utf-8") as f:
    text = f.read()
    
# Replace the old beaconSpamLoop entirely
new_loop = """#include "input_manager.h"

void beaconSpamLoop() {
    Input.update();
    
    if (Input.pressed(BTN_ID_OK) || Input.pressed(BTN_ID_AUX)) {
        isSpamming = !isSpamming;
        if (isSpamming) {
            WifiHelper::setupPromiscuous();
        } else {
            WifiHelper::teardownPromiscuous();
        }
    }

    if (Input.pressed(BTN_ID_BACK) || Input.cancelRequested()) {
        if (isSpamming) {
            isSpamming = false;
            WifiHelper::teardownPromiscuous();
        }
        extern void exitCurrentApp();
        exitCurrentApp();
        return;
    }

    if (isSpamming) {
        for (int i = 0; i < total_ssids; i++) {
            // --- MEJORA BYPASS IPHONE 13: MAC ALEATORIA ---
            packet[10] = 0x00; packet[11] = 0x16; packet[12] = 0xEA; // Vendor Intel/Cisco
            packet[13] = random(0, 255); packet[14] = random(0, 255); packet[15] = (uint8_t)i;
            memcpy(&packet[16], &packet[10], 6); // BSSID = Source

            // SSID
            int len = strlen(ssids[i]);
            packet[36] = 0x00; packet[37] = len; 
            for (int j = 0; j < len; j++) packet[38 + j] = ssids[i][j]; 
            
            int p = 38 + len;
            // Rates (Crucial para iPhone)
            packet[p++] = 0x01; packet[p++] = 0x08;
            packet[p++] = 0x82; packet[p++] = 0x84; packet[p++] = 0x8b; packet[p++] = 0x96;
            packet[p++] = 0x24; packet[p++] = 0x30; packet[p++] = 0x48; packet[p++] = 0x6c;

            // Canal
            packet[p++] = 0x03; packet[p++] = 0x01;
            int ch = random(1, 11); // Evitar canales altos problemAticos
            packet[p++] = (uint8_t)ch;

            // --- MEJORA BYPASS IPHONE 13: RSN (WPA2) ---
            packet[p++] = 0x30; packet[p++] = 0x14; 
            packet[p++] = 0x01; packet[p++] = 0x00; packet[p++] = 0x00; packet[p++] = 0x0f;
            packet[p++] = 0xac; packet[p++] = 0x04; packet[p++] = 0x01; packet[p++] = 0x00;
            packet[p++] = 0x00; packet[p++] = 0x0f; packet[p++] = 0xac; packet[p++] = 0x04;
            packet[p++] = 0x01; packet[p++] = 0x00; packet[p++] = 0x00; packet[p++] = 0x0f;
            packet[p++] = 0xac; packet[p++] = 0x02; packet[p++] = 0x00; packet[p++] = 0x00;

            esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
            esp_wifi_80211_tx(WIFI_IF_STA, packet, p, false);
            
            // Allow input processing during spamming
            Input.update();
            if (Input.pressed(BTN_ID_BACK) || Input.cancelRequested() || Input.pressed(BTN_ID_OK)) {
                isSpamming = false;
                WifiHelper::teardownPromiscuous();
                if (Input.pressed(BTN_ID_BACK) || Input.cancelRequested()) {
                    extern void exitCurrentApp();
                    exitCurrentApp();
                }
                return;
            }
        }
        
        // --- GESTION DE ANIMACION ---
        if (millis() - lastAnimUpdate > 100) {
            lastAnimUpdate = millis();
            animFrame = (animFrame + 1) % 4;
            u8g2.clearBuffer();
            drawBeaconActive(animFrame);
            u8g2.sendBuffer(); oledMirrorSync();
        }

    } else {
        u8g2.clearBuffer();
        drawBeaconIdle();
        u8g2.sendBuffer(); oledMirrorSync();
    }
}"""

import re
text = re.sub(r"void beaconSpamLoop\(\) \{.*$", new_loop, text, flags=re.DOTALL)

with open("src/beacon_spam.cpp", "w", encoding="utf-8") as f:
    f.write(text)

