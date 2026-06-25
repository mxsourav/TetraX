#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <RF24.h>
#include <ESP32Ping.h>
#include <AsyncUDP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <esp_wifi.h>

#include "spectrograph.h"
#include "rf_heatmap.h"
#include "channel_advisor.h"
#include "coex_view.h"
#include "dual_nrf_scope.h"
#include "nrf_link.h"
#include "nrf_chat.h"
#include "jammer.h"
#include "wifiscan.h"
#include "total_jammer.h"
#include "btscan.h"
#include "bt_jammer.h"
#include "beacon_spam.h"
#include "hybrid_attack.h"
#include "ip_scanner.h"
#include "snake_game.h"
#include "packet_monitor.h"
#include "gui_helper.h"
#include "slave_control.h"
#include "wifiscan.h"
#include "total_jammer.h"
#include "btscan.h"
#include "bt_jammer.h"
#include "beacon_spam.h"
#include "hybrid_attack.h"
#include "ip_scanner.h"
#include "snake_game.h"
#include "packet_monitor.h"
#include "gui_helper.h"
#include "slave_control.h"
#include "web_dashboard.h"
#include "about_info.h"
#include "evil_portal.h"
#include "web_server.h"
#include "log_viewer.h"
#include "ble_spam.h"
#include "app_config.h"
#include "input_manager.h"
#include "app_lifecycle.h"
#include "sd_manager.h"
#include "ir_manager.h"
#include "app_ir_remote.h"
#include "app_ir_receiver.h"
#include "app_ir_jammer.h"
#include "app_ir_sd.h"
#include "bt_remote.h"
#include "bt_analyzer.h"
#include "bt_spectrum.h"
#include "centinela.h"
#include "games_menu.h"
#include "ui_theme.h"
#include "ip_scanner.h"
#include "wifiscan.h"
#include "wifi_radar.h"
#include "wifi_channel_scan.h"
#include "menu_catalog.h"
#include "Deauther.h"
#include "oled_mirror.h"
void totalJammerSetup(); void totalJammerLoop();
void btscanSetup(); void btscanLoop();
void btJammerSetup(); void btJammerLoop();
void beaconSpamLoop(); void bleSpamEnter(); void bleSpamLoop(); void bleSpamExit();
void hybridAttackLoop();
void evilPortalLoop(); void logViewerLoop();
void startWebServer();
void btRemoteLoop();
void btAnalyzerEnter(); void btAnalyzerLoop(); void btAnalyzerExit();
void centinelaLoop();
void wifiscanEnter(); void wifiscanLoop(); void wifiscanExit();

// --- VARIABLES DE ESTADO (idénticas al original) ---
RTC_DATA_ATTR bool forceBleMode = false;
bool isTotalAttacking = false;
bool isHybridActive = false;
String target_ssid = "Ninguna";
int target_channel = 1;

// Constructores literales — NO los reorganizamos para no romper el wiring
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 22, 21);
RF24 jam1(5, 17, 16000000);
RF24 jam2(16, 4, 16000000);

AsyncWebServer asyncServer(AppConfig::WEB_PORT);

int   menu_index   = 0;
int   category_index = 0;
int   category_app_index = 0;
bool  browsingCategoryApps = false;
float currentPos   = 0;
bool  runningApp   = false;
int   TOTAL_OPTIONS = 33;

// --- ETIQUETAS DEL MENÚ (idénticas) ---
const char* menu_labels[] = {
    "WIFI SCANNER",    // 0
    "WIFI RADAR",      // 1
    "CHANNEL SCAN",    // 2
    "ANALIZADOR",      // 3
    "BT SCANNER",      // 4
    "PACKET MONITOR",  // 5
    "MODO CENTINELA",  // 6
    "JAMMER CANAL",    // 7
    "BARRIDO TOTAL",   // 8
    "BT JAMMER",       // 9
    "BEACON SPAM",     // 10
    "BLE SPAM (POP)",  // 11
    "MODO HIBRIDO",    // 12
    "EVIL PORTAL",     // 13
    "IP SCANNER",      // 14
    "CONTROL ESCLAVO", // 15
    "WEB DASHBOARD",   // 16
    "IR REMOTE",       // 17
    "LEER LOGS",       // 18
    "ARCADE",          // 19
    "ABOUT",           // 20
    "BT ANALYZER",     // 21
    "BT SPECTRUM",     // 22
    "RF HEATMAP",      // 23
    "CH ADVISOR",      // 24
    "NRF LINK",        // 25
    "NRF CHAT",        // 26
    "BT/WIFI COEX",    // 27
    "DUAL NRF SCOPE",  // 28
    "DEAUTHER",        // 29
    "IR RECEIVER",     // 30
    "IR JAMMER",       // 31
    "FROM SD CARD"     // 32
};

static void deautherLoop() {
    runDeauther();
    runningApp = false;
}

// =============================================================
// Tabla de apps: { name, enter(), loop(), exit() }
// - enter()  = lo que era xxxSetup() en el cascade original
// - loop()   = lo que era xxxLoop() en el cascade original
// - exit()   = nullptr por ahora; los .cpp de cada módulo NO se modifican
//
// El comportamiento interno de cada módulo queda intacto. Esta tabla
// solo cambia el DESPACHO desde main.cpp, no la lógica de los módulos.
// =============================================================
static const App apps[] = {
    /*  0 */ { "WIFI SCANNER",    wifiscanEnter,      wifiscanLoop,      wifiscanExit },
    /*  1 */ { "WIFI RADAR",      wifiRadarEnter,     wifiRadarLoop,     wifiRadarExit },
    /*  2 */ { "CHANNEL SCAN",    wifiChannelScanEnter, wifiChannelScanLoop, wifiChannelScanExit },
    /*  3 */ { "ANALIZADOR",      spectrographEnter,  spectrographLoop,  spectrographExit },
    /*  4 */ { "BT SCANNER",      btscanSetup,        btscanLoop,        btscanExit },
    /*  5 */ { "PACKET MONITOR",  monitorEnter,       monitorLoop,       monitorExit      },
    /*  6 */ { "MODO CENTINELA",  centinelaEnter,     centinelaLoop,     centinelaExit    },
    /*  7 */ { "JAMMER CANAL",    jammerSetup,        jammerLoop,        nullptr },
    /*  8 */ { "BARRIDO TOTAL",   totalJammerSetup,   totalJammerLoop,   nullptr },
    /*  9 */ { "BT JAMMER",       btJammerSetup,      btJammerLoop,      nullptr },
    /* 10 */ { "BEACON SPAM",     nullptr,            beaconSpamLoop,    nullptr },
    /* 11 */ { "BLE SPAM (POP)",  bleSpamEnter,       bleSpamLoop,       bleSpamExit },
    /* 12 */ { "MODO HIBRIDO",    nullptr,            hybridAttackLoop,  nullptr },
    /* 13 */ { "EVIL PORTAL",     nullptr,            evilPortalLoop,    nullptr },
    /* 14 */ { "IP SCANNER",      ipScannerEnter,     ipScannerLoop,     ipScannerExit    },
    /* 15 */ { "CONTROL ESCLAVO", slaveControlEnter,  slaveControlLoop,  slaveControlExit },
    /* 16 */ { "WEB DASHBOARD",   startWebServer,     webDashboardLoop,  nullptr },
    /* 17 */ { "IR REMOTE",       irRemoteEnter,      irRemoteLoop,      irRemoteExit },
    /* 18 */ { "LEER LOGS",       nullptr,            logViewerLoop,     nullptr },
    /* 19 */ { "ARCADE",          gamesEnter,         gamesLoop,         gamesExit },
    /* 20 */ { "ABOUT",           aboutEnter,         aboutLoop,         aboutExit },
    /* 21 */ { "BT ANALYZER",     btAnalyzerEnter,    btAnalyzerLoop,    btAnalyzerExit },
    /* 22 */ { "BT SPECTRUM",     btSpectrumEnter,    btSpectrumLoop,    btSpectrumExit },
    /* 23 */ { "RF HEATMAP",      rfHeatmapEnter,     rfHeatmapLoop,     rfHeatmapExit },
    /* 24 */ { "CH ADVISOR",      channelAdvisorEnter, channelAdvisorLoop, channelAdvisorExit },
    /* 25 */ { "NRF LINK",        nrfLinkEnter,       nrfLinkLoop,       nrfLinkExit },
    /* 26 */ { "NRF CHAT",        nrfChatEnter,       nrfChatLoop,       nrfChatExit },
    /* 27 */ { "BT/WIFI COEX",    coexViewEnter,      coexViewLoop,      coexViewExit },
    /* 28 */ { "DUAL NRF SCOPE",  dualNrfScopeEnter,  dualNrfScopeLoop,  dualNrfScopeExit },
    /* 29 */ { "DEAUTHER",        nullptr,            deautherLoop,      nullptr },
    /* 30 */ { "IR RECEIVER",     irReceiverEnter,    irReceiverLoop,    irReceiverExit },
    /* 31 */ { "IR JAMMER",       irJammerEnter,      irJammerLoop,      irJammerExit },
    /* 32 */ { "FROM SD CARD",    irSdEnter,          irSdLoop,          irSdExit },
};
static const uint8_t APPS_COUNT = sizeof(apps) / sizeof(App);


static bool taskCancelInProgress = false;

static void performBackCleanup() {
    if (taskCancelInProgress) {
        Serial.println("[CLEANUP] Cleanup already in progress, aborting recursive call.");
        return;
    }

    taskCancelInProgress = true;
    Serial.println("[CLEANUP] Executing strict cleanup sequence...");

    // 1. disable promiscuous callbacks
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(nullptr);
    
    // 2. stop timers/tasks & 3. stop scans/attacks
    jam1.stopConstCarrier();
    jam2.stopConstCarrier();
    if (menu_index == 16) asyncServer.end();

    // 4. release SPI/radios
    bool keepWifiConnection = (menu_index == 0 && WiFi.status() == WL_CONNECTED);
    if (!keepWifiConnection) {
        WiFi.mode(WIFI_OFF);
    }

    Serial.println("[CLEANUP] Global cleanup complete.");
    taskCancelInProgress = false;
}


static bool needsRestartAfterExit(int idx) {
        return false;
}

void setup() {
    Serial.begin(115200);
    u8g2.begin();

    Input.begin();
    Host.registerApps(apps, APPS_COUNT);

    for (int i = 0; i <= 100; i += 5) {
        UiTheme::drawSplashFrame(u8g2, i, i / 5);
        u8g2.sendBuffer(); oledMirrorSync();
        delay(35);
    }

    // Initialize shared VSPI and set default CS states for coexistence
    pinMode(AppConfig::SD_CS, OUTPUT);
    digitalWrite(AppConfig::SD_CS, HIGH); // Disable SD
    pinMode(AppConfig::NRF1_CSN, OUTPUT);
    digitalWrite(AppConfig::NRF1_CSN, HIGH); // Disable NRF1
    pinMode(AppConfig::NRF2_CSN, OUTPUT);
    digitalWrite(AppConfig::NRF2_CSN, HIGH); // Disable NRF2
    
    SPI.begin(18, 19, 23); // SCK, MISO, MOSI

    // Inicialización temprana idéntica al original
    jam1.begin();
    jam2.begin();

    // Init IR Transceiver
    IRManager::getInstance().begin();

    // Init SD Card
    if (SDManager::getInstance().begin()) {
        SDManager::getInstance().runIsolatedTest();
    }
    
    Serial.println("[BWIFIKILL_V4_CONNECTED]");
}

unsigned long lastTelemetryMs = 0;

void loop() {
    if (millis() - lastTelemetryMs >= 1000) {
        lastTelemetryMs = millis();
        Serial.printf("[UPTIME] %lu\n", millis() / 1000);
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[NET] STA_IP=%s\n", WiFi.localIP().toString().c_str());
        } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
            Serial.printf("[NET] AP_IP=%s\n", WiFi.softAPIP().toString().c_str());
        }
    }


    // Procesamiento de comandos Serial
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        
        if (cmd.startsWith("BTN_")) {
            bool pressed = cmd.endsWith("_PRESS");
            String btnStr = cmd.substring(4, cmd.lastIndexOf('_'));
            ButtonId id = BTN_COUNT;
            if (btnStr == "UP") id = BTN_ID_UP;
            else if (btnStr == "DOWN") id = BTN_ID_DOWN;
            else if (btnStr == "LEFT") id = BTN_ID_LEFT;
            else if (btnStr == "RIGHT") id = BTN_ID_RIGHT;
            else if (btnStr == "OK") id = BTN_ID_OK;
            else if (btnStr == "BACK") id = BTN_ID_BACK;
            else if (btnStr == "AUX") id = BTN_ID_AUX;
            
            if (id != BTN_COUNT) Input.injectVirtualState(id, pressed);
        } else if (cmd.startsWith("CMD_OPEN_")) {
            int targetApp = -1;
            if (cmd == "CMD_OPEN_IR_JAMMER") targetApp = 7;
            else if (cmd == "CMD_OPEN_IR_REMOTE") targetApp = 17;
            else if (cmd == "CMD_OPEN_IR_RECEIVER") targetApp = 30;
            else if (cmd == "CMD_OPEN_WIFI_SCAN") targetApp = 0;
            else if (cmd == "CMD_OPEN_WIFI_RADAR") targetApp = 1;
            
            if (targetApp != -1) {
                Host.launch(targetApp);
                runningApp = true;
            }
        }
    }

    Input.update();

    if (!runningApp) {
        int old_category = category_index;

        // --- NAVEGACIÓN DEL MENÚ ---
        // repeating() = primer pulso + auto-repeat fluido (reemplaza delay(200))
        if (Input.repeating(BTN_ID_UP)) {
            if (browsingCategoryApps) {
                const MenuCategory& cat = menuCategoryAt(category_index);
                category_app_index--;
                if (category_app_index < 0) category_app_index = cat.count - 1;
                menu_index = menuCategoryAppIndex(category_index, category_app_index);
            } else {
                category_index--;
                if (category_index < 0) category_index = menuCategoryCount() - 1;
            }
        }
        if (Input.repeating(BTN_ID_DOWN)) {
            if (browsingCategoryApps) {
                const MenuCategory& cat = menuCategoryAt(category_index);
                category_app_index++;
                if (category_app_index >= cat.count) category_app_index = 0;
                menu_index = menuCategoryAppIndex(category_index, category_app_index);
            } else {
                category_index++;
                if (category_index >= menuCategoryCount()) category_index = 0;
            }
        }

        if (category_index != old_category) {
            if (category_index == 5) {
                IRManager::getInstance().beginSession();
            } else if (old_category == 5) {
                IRManager::getInstance().endSession();
            }
        }

        drawBruceMenu();

        if (Input.pressed(BTN_ID_OK)) {
            if (!browsingCategoryApps) {
                browsingCategoryApps = true;
                category_app_index = 0;
                menu_index = menuCategoryAppIndex(category_index, category_app_index);
                delay(180);
            } else {
                menu_index = menuCategoryAppIndex(category_index, category_app_index);
                runningApp = true;
            Host.launch(menu_index);   // llama enter() si está definido
                delay(300);                 // preserva debounce de entrada del original
            }
        }

        if (Input.pressed(BTN_ID_BACK) && browsingCategoryApps) {
            browsingCategoryApps = false;
            delay(180);
        }
    } else {
        // --- EJECUCIÓN ---
        Host.tick();   // llama loop() del módulo actual

        // CASO A: el módulo se salió desde adentro (puso runningApp = false).
        // Algunos módulos (about, ble_spam en menu, bt_remote, slave_control...)
        // lo hacen vía su propio digitalRead(25). Aquí sincronizamos el Host
        // y hacemos la misma limpieza que el handler de BACK.
        if (!runningApp && Host.isRunning()) {
            Host.shutdown();
            performBackCleanup();
            if (needsRestartAfterExit(menu_index)) {
                ESP.restart();
            }
            return;
        }

        // CASO B: BACK manejado desde main.cpp (fallback).
        // Misma excepción de siempre: IP Scanner (14) maneja su propio BACK.
        if (Input.pressed(BTN_ID_BACK) && menu_index != 14) {
            runningApp = false;
            Host.shutdown();
            performBackCleanup();
            delay(300);
            if (needsRestartAfterExit(menu_index)) {
                ESP.restart();
            }
        }
    }
}
