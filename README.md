<div align="center">

# TetraX

### Advanced Multi-Protocol RF Diagnostic & Network Auditing Subsystem

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![C](https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=black)
![PlatformIO](https://img.shields.io/badge/PlatformIO-FF7F00?style=for-the-badge&logo=platformio&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino_Framework-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![HTML5](https://img.shields.io/badge/HTML5-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)
![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white)
![React](https://img.shields.io/badge/React-61DAFB?style=for-the-badge&logo=react&logoColor=black)
![TypeScript](https://img.shields.io/badge/TypeScript-3178C6?style=for-the-badge&logo=typescript&logoColor=white)
![License](https://img.shields.io/badge/License-Restrictive_Use-red?style=for-the-badge)

**Developer: mx_sourav**

*A hardware-accelerated, multi-vector RF analysis platform built on the ESP32-WROOM-32 SoC, providing raw access to the 802.11 MAC sublayer, BLE Link Layer, and 2.4GHz ISM band through NRF24L01+ transceiver integration.*

---

</div>

## Architecture Overview

```
TetraX Firmware V5.0 — System Block Diagram
============================================

                   ┌─────────────────────────────────────────────────────┐
                   │                  ESP32-WROOM-32                     │
                   │          Dual-Core Xtensa LX6 @ 240MHz              │
                   │              320KB SRAM / 4MB Flash                 │
                   │                                                     │
                   │  ┌──────────┐  ┌──────────┐  ┌──────────────────┐   │
                   │  │ WiFi PHY │  │  BT/BLE  │  │  Application     │   │
                   │  │ 802.11   │  │  4.2 LE  │  │  Core (C++)      │   │
                   │  │ b/g/n    │  │  GAP/ATT │  │                  │   │
                   │  └────┬─────┘  └────┬─────┘  │  ┌────────────┐  │   │
                   │       │             │        │  │ UI Engine  │  │   │
                   │  ┌────┴─────────────┴────┐   │  │ U8g2/OLED  │  │   │
                   │  │   Patched libnet80211 │   │  └────────────┘  │   │
                   │  │   (Raw Frame Inject)  │   │  ┌────────────┐  │   │
                   │  └───────────────────────┘   │  │ Input Mgr  │  │   │
                   │                              │  │ 5-Button   │  │   │
                   │  ┌───────────────────────┐   │  └────────────┘  │   │
                   │  │   SPI Bus (VSPI)      │   │  ┌────────────┐  │   │
                   │  │   NRF24L01+ | SD Card │   │  │ IR Engine  │  │   │
                   │  └───────────────────────┘   │  │ TX/RX 38K  │  │   │
                   │                              │  └────────────┘  │   │
                   │  ┌───────────────────────┐   │  ┌────────────┐  │   │
                   │  │   UART0 (USB/Flash)   │   │  │ RAONE Link │  │   │
                   │  │   GPIO 1 TX / 3 RX    │   │  │ RX2:16 TX2:17││   │
                   │  └───────────────────────┘   │  └────────────┘  │   │
                   │                              └──────────────────┘   │
                   └─────────────────────────────────────────────────────┘
                          │          │           │           │
                     ┌────┴───┐ ┌───┴────┐ ┌───┴────┐ ┌───┴─────┐
                     │SSD1306 │ │NRF24L01│ │SD Card │ │RAONE 5G │
                     │128x64  │ │  + PA  │ │ SPI    │ │BW16 Link│
                     │ OLED   │ │        │ │ FAT32  │ │Serial2  │
                     └────────┘ └────────┘ └────────┘ └─────────┘
```

---

## Complete Feature Matrix

### WiFi Subsystem (802.11 b/g/n)

| Feature | Technical Implementation | Use Case |
|---|---|---|
| **Promiscuous Packet Capture** | `esp_wifi_set_promiscuous(true)` with custom RX callback. Drops MAC address filter at the hardware level, capturing all 802.11 Management, Control, and Data frames within the 2.4GHz ISM band regardless of destination BSSID. | Passive network topology mapping, hidden SSID discovery, client-AP association tracking |
| **802.11 Deauthentication** | Constructs IEEE 802.11 Class 3 Management frames (subtype 0x0C) with spoofed source MAC. Bypasses Espressif's `ieee80211_raw_frame_sanity_check` via statically patched `libnet80211.a` to allow injection of arbitrary management frames. | WPA/WPA2 handshake capture facilitation, network stability stress testing |
| **Beacon Frame Injection** | Generates synthetic 802.11 Beacon frames (subtype 0x08) containing crafted SSID Information Elements, Supported Rates, and DS Parameter Set IEs. Cycles through configurable SSID lists at high frame rates. | Client-side wireless stack robustness testing, SSID enumeration resilience analysis |
| **Evil Portal / Captive Portal** | Deploys a SoftAP with DNS hijacking (all DNS queries resolve to `192.168.4.1`) and serves customizable credential capture pages. Logs are written to SD card in plaintext. | Phishing awareness demonstrations, social engineering audit |
| **WiFi Channel Scanner** | Iterates channels 1-14 using `esp_wifi_set_channel()`, capturing beacon frames per-channel and computing RSSI histograms. Renders real-time spectral occupancy on the OLED. | RF site survey, channel interference analysis |
| **WiFi Radar** | Continuous promiscuous mode scanning with per-AP RSSI tracking over time. Renders proximity-based radar visualization. | Physical AP localization, rogue AP detection |
| **Packet Monitor** | Real-time frame rate counter segmented by frame type (Management/Data/Control). Renders throughput graphs on OLED. | Network load analysis, anomaly detection |
| **IP Scanner** | ARP-based host discovery on the local subnet with parallel ICMP Echo (ping) probes. Batch size configurable. | Network inventory, connected device enumeration |
| **Sentinel Mode** | Background watchdog monitoring new AP appearances or disappearances against a baseline snapshot. | Unauthorized AP detection, continuous network integrity monitoring |

### Bluetooth / BLE Subsystem

| Feature | Technical Implementation | Use Case |
|---|---|---|
| **BLE Advertisement Flooding** | Crafts raw BLE GAP advertising payloads mimicking Apple Continuity (AirPods/AirDrop proximity), Microsoft Swift Pair, Google Fast Pair, and Samsung BLE protocols. Rapidly cycles advertising data on channels 37/38/39. | BLE stack resilience testing, notification saturation assessment |
| **BLE Device Scanner** | Standard BLE scan using `BLEDevice::getScan()` with active scan mode. Reports device name, MAC, RSSI, manufacturer data, and service UUIDs. | BLE device inventory, proximity analysis |
| **BT Classic Analyzer** | Deep inspection of Bluetooth Classic inquiry results including CoD (Class of Device) decoding, EIR (Extended Inquiry Response) parsing, and service discovery. | Bluetooth topology mapping, device classification |
| **BT Spectrum Analyzer** | Visualizes the 2.4GHz BLE advertising channel energy levels using rapid sequential scans with RSSI aggregation. | ISM band congestion measurement, co-channel interference mapping |
| **BT HID Keyboard** | Registers as a Bluetooth HID peripheral (keyboard) and injects keystroke sequences to paired devices. Supports macro playback and URL auto-typing. | USB Rubber Ducky-style payload delivery, automated input testing |

### NRF24L01+ Sub-GHz Module

| Feature | Technical Implementation | Use Case |
|---|---|---|
| **Continuous Wave Jammer** | Forces the NRF24 PLL into unmodulated carrier-wave (CW) mode by writing directly to hardware registers, bypassing the packet handler entirely. Produces a continuous RF carrier on a selected 1MHz channel in the 2.4GHz band. | WiFi/BT coexistence testing, receiver desensitization measurement |
| **RF Spectrum Analyzer** | Rapid channel sweep across all 126 NRF24 channels (2400-2525MHz) measuring received signal strength. Renders spectral waterfall and peak-hold displays on OLED. | ISM band occupancy visualization, interference source identification |
| **RF Heatmap** | Aggregates NRF24 channel energy readings over time to build a 2D spectral heatmap showing persistent vs transient signals. | Long-term RF environment characterization |
| **NRF Chat** | Bidirectional text messaging over NRF24 radio link between two TetraX units. Uses ShockBurst protocol with auto-acknowledgment. | Secure off-grid communication, field team coordination |
| **NRF Link** | High-speed data transfer between two NRF24-equipped devices with CRC verification and retry logic. | File transfer over RF, telemetry relay |
| **Dual NRF Scope** | Simultaneous dual-channel NRF monitoring when two NRF24 modules are attached. | Cross-channel correlation analysis |

### IR Subsystem

| Feature | Technical Implementation | Use Case |
|---|---|---|
| **IR Remote** | Transmits pre-programmed IR codes for common consumer devices (TV power, volume, channel) using standard protocols (NEC, Sony, RC5, Samsung). | Universal remote functionality, device control testing |
| **IR Jammer** | Floods the 38KHz IR carrier with continuous modulated noise to prevent IR receivers from decoding legitimate commands. | IR receiver robustness testing |
| **IR Receiver** | Captures and decodes incoming IR signals, displaying protocol, address, and command data. Captured codes can be saved to SD card. | IR protocol reverse engineering, remote control cloning |
| **IR SD Replay** | Replays previously captured IR signals from SD card storage. | Automated IR sequence playback |

### Hardware Master/Slave Expansion

| Feature | Technical Implementation | Use Case |
|---|---|---|
| **RAONE Master-Slave Link** | Dedicated Hardware Serial2 (`RX2: GPIO 16`, `TX2: GPIO 17` @ 115200 baud) for high-speed bi-directional telemetry and remote attack orchestration with the RAONE BW16 5GHz co-processor. | Coordinated dual-band operations, 5GHz offloading, remote attack execution |
| **Dual-Band Deauthentication** | TetraX executes local 2.4GHz frame injection while instructing RAONE over UART to conduct 5GHz deauthentication simultaneously. *(Implemented in firmware — pending field verification)*. | Dual-band wireless infrastructure stress testing |
| **UART Remote Keypad** | Forwards navigation keystrokes (`NAV`, `OK`, `BACK`) to external slave controllers or receives remote commands. | Remote terminal control, headless operation |
| **Slave Manager** | Protocol handler for incoming UART commands with background heartbeat parser and hot-plug reconnect state machine. | Automated test orchestration, multi-device mesh |

### Web Dashboard & SATAN Interface

TetraX can be monitored and controlled via two distinct interfaces:
1. **Local SoftAP Dashboard**: Connect to the TetraX_V4 WiFi network and navigate to 192.168.4.1 for the local OLED mirror and basic controls.
2. **SATAN UI Integration**: TetraX fully supports the **SATAN** (Security Analysis and Tactical Attack Network) web interface. You can access the advanced web controller at: **[espsatan.vercel.app](https://espsatan.vercel.app)**. Connect your TetraX to your PC via USB, open the SATAN web app in a WebSerial-compatible browser (like Chrome/Edge), and take full command of the hardware via the serial link.

### System Utilities

| Feature | Description |
|---|---|
| **OLED Mirror (Web)** | Real-time OLED display mirrored to a web dashboard over WiFi SoftAP. Accessible via browser at `192.168.4.1`. |
| **SD Card Manager** | FAT32 file browser with create, read, delete operations. Stores IR captures, Evil Portal logs, and configuration files. |
| **Idle Animation** | Animated idle screen with reactive companion character and system status display. |
| **Mini Games** | Built-in Dino Runner, Flappy Bird, Snake, Pong, and Space Invaders for OLED. |
| **Channel Advisor** | Recommends the least congested WiFi channel based on live spectral scan data. |
| **About / System Info** | Displays firmware version, memory usage, uptime, and developer credits. |

---

## Hardware Pin Map

### ESP32 GPIO Allocation Table

| GPIO | Function | Direction | Protocol | Notes |
|------|----------|-----------|----------|-------|
| **0** | BOOT Button (External) | INPUT | Digital | Active LOW. Wire to external momentary switch + GND for firmware flashing from enclosure. Directly controls the ESP32 strapping pin — pulling LOW during power-on enters download mode. |
| **1** | UART0 TX / USB Port | OUTPUT | UART | Dedicated USB Serial TX for firmware flashing and debug monitor output. |
| **2** | IR Transmitter | OUTPUT | PWM/38KHz | Directly drives IR LED. This is an ESP32 strapping pin — firmware forces LOW immediately on boot to prevent phantom IR transmission during power-on. |
| **3** | UART0 RX / USB Port | INPUT | UART | Dedicated USB Serial RX for firmware flashing. |
| **4** | Buzzer | OUTPUT | Digital | Active buzzer, driven with 5ms synchronous pulses for tactile click feedback. |
| **5** | NRF24 CE (Chip Enable) | OUTPUT | SPI/Digital | Controls the NRF24L01+ transmit/receive enable. Active HIGH to begin TX or enter RX mode. |
| **12** | NRF24 CSN (SPI Select) | OUTPUT | SPI (VSPI) | Active LOW chip select for the NRF24L01+ transceiver. |
| **13** | SD Card CS | OUTPUT | SPI (VSPI) | Directly drives the SD module CS. Shared with the NRF24 on the same VSPI bus. Bus arbitration handled in software. |
| **16** | RX2 (Serial2 RX) | INPUT | UART2 | Dedicated high-speed UART input for RAONE BW16 5GHz co-processor (connects to RAONE PA7/LOG_TX). |
| **17** | TX2 (Serial2 TX) | OUTPUT | UART2 | Dedicated high-speed UART output for RAONE BW16 5GHz co-processor (connects to RAONE PA8/LOG_RX). |
| **18** | SPI SCK | OUTPUT | SPI (VSPI) | Shared SPI clock for NRF24L01+ and SD Card module. Hardware VSPI default. |
| **19** | SPI MISO | INPUT | SPI (VSPI) | Shared SPI data input from NRF24L01+ and SD Card module. |
| **21** | I2C SDA | BIDIR | I2C | Data line to SSD1306 OLED display. Internal pull-up enabled. |
| **22** | I2C SCL | OUTPUT | I2C | Clock line to SSD1306 OLED display. Internal pull-up enabled. |
| **23** | SPI MOSI | OUTPUT | SPI (VSPI) | Shared SPI data output to NRF24L01+ and SD Card module. |
| **25** | Button BACK | INPUT | Digital | Internal pull-up. Active LOW. Directly mapped to UI navigation. |
| **26** | Button UP | INPUT | Digital | Internal pull-up. Active LOW. Scroll/navigate up in menus. |
| **27** | Button DOWN | INPUT | Digital | Internal pull-up. Active LOW. Scroll/navigate down in menus. |
| **32** | Button OK/SELECT | INPUT | Digital | Internal pull-up. Active LOW. Confirm selection, start actions. |
| **33** | Button AUX | INPUT | Digital | Internal pull-up. Active LOW. Auxiliary/context-specific actions. |
| **35** | IR Receiver | INPUT | Digital/38KHz | Input-only GPIO. Connected to IR receiver module (TSOP38238 or equivalent). Decodes 38KHz modulated IR signals. |

### External USB Female Port Wiring

This port serves firmware flashing (via USB-to-TTL converter) and serial debugging.

| USB Port Pin | ESP32 Connection | USB-to-TTL Wire | Notes |
|---|---|---|---|
| **VCC (Pin 1)** | 5V / VIN | VCC (Red) | Powers the ESP32 when external power is connected. |
| **D- (Pin 2)** | GPIO 1 (TX0) | RXD (White/Yellow) | ESP32 TX connects to TTL adapter RX. Cross-wired. |
| **D+ (Pin 3)** | GPIO 3 (RX0) | TXD (Green) | ESP32 RX connects to TTL adapter TX. Cross-wired. |
| **GND (Pin 4)** | GND | GND (Black) | Common ground between ESP32 and USB-to-TTL adapter. |

**Flashing procedure**: Connect USB-to-TTL adapter, hold the external BOOT button (GPIO 0 to GND), press RST (power cycle), release BOOT, then run `pio run -t upload`.

### External Boot Button Wiring

| Component | Connection | Notes |
|---|---|---|
| Button Terminal 1 | GPIO 0 | Direct connection to ESP32 GPIO 0 (strapping pin) |
| Button Terminal 2 | GND | Common ground |

When pressed during power-on/reset, GPIO 0 is pulled LOW, forcing the ESP32 into UART download mode for firmware flashing.

### SD Card Module (SPI)

| SD Module Pin | ESP32 GPIO | SPI Function |
|---|---|---|
| CS | GPIO 13 | Chip Select (directly managed) |
| MOSI | GPIO 23 | SPI Data Out (shared VSPI bus) |
| MISO | GPIO 19 | SPI Data In (shared VSPI bus) |
| SCK | GPIO 18 | SPI Clock (shared VSPI bus) |
| VCC | 3.3V | Power supply |
| GND | GND | Common ground |

### NRF24L01+ Transceiver Module (SPI)

| NRF24 Pin | ESP32 GPIO | SPI Function |
|---|---|---|
| CE | GPIO 5 | Chip Enable (TX/RX activation) |
| CSN | GPIO 12 | SPI Chip Select (active LOW) |
| MOSI | GPIO 23 | SPI Data Out (shared VSPI bus) |
| MISO | GPIO 19 | SPI Data In (shared VSPI bus) |
| SCK | GPIO 18 | SPI Clock (shared VSPI bus) |
| VCC | 3.3V | Power supply (use capacitor for stability) |
| GND | GND | Common ground |

### Master/Slave UART Expansion (RAONE BW16 5GHz Co-Processor / External)

| Serial Interface | ESP32 GPIO | RAONE BW16 Pin | Function |
|---|---|---|---|
| **RX2 (Serial2 RX)** | **GPIO 16** | **PA7 (LOG_TX)** | Receives live 5GHz telemetry, heartbeats, and status reports |
| **TX2 (Serial2 TX)** | **GPIO 17** | **PA8 (LOG_RX)** | Transmits remote attack commands & keypad navigation |
| **GND** | **GND** | **GND** | Common ground reference plane (mandatory) |

**Hot-Plug Handshake**: TetraX continuously listens on Serial2 (`RX2: GPIO 16`, `TX2: GPIO 17`). Upon receiving telemetry frames or heartbeats from RAONE, TetraX instantly locks the connection and enables the dual-band attack suite without requiring synchronous reboots.

### SSD1306 OLED Display (I2C)

| OLED Pin | ESP32 GPIO | I2C Function |
|---|---|---|
| SDA | GPIO 21 | I2C Data |
| SCL | GPIO 22 | I2C Clock |
| VCC | 3.3V | Power supply |
| GND | GND | Common ground |

### IR Module

| IR Component | ESP32 GPIO | Notes |
|---|---|---|
| IR LED (Transmitter) | GPIO 2 | Series resistor recommended. Firmware forces LOW on boot to prevent phantom transmission during power-on reset. |
| IR Receiver (TSOP) | GPIO 35 | Input-only pin. 38KHz bandpass demodulator output. |

### Navigation Buttons

| Button | ESP32 GPIO | Function |
|---|---|---|
| UP | GPIO 26 | Navigate up / scroll |
| DOWN | GPIO 27 | Navigate down / scroll |
| OK | GPIO 32 | Select / confirm / start |
| BACK | GPIO 25 | Go back / cancel / exit |
| AUX | GPIO 33 | Contextual auxiliary action |

### Active Buzzer

| Buzzer Pin | ESP32 GPIO | Notes |
|---|---|---|
| Signal | GPIO 4 | Active buzzer. Driven with 5ms synchronous pulses for click feedback. |
| GND | GND | Common ground |

---

## Technical Deep Dive

### 802.11 Raw Frame Injection Pipeline

Standard ESP-IDF enforces sanity checks on all outgoing 802.11 frames through `ieee80211_raw_frame_sanity_check()` inside the proprietary `libnet80211.a` blob. This function validates frame type, subtype, and certain IE constraints before allowing transmission. TetraX patches this restriction at the binary level:

1. **Symbol Weakening**: The `xtensa-esp32-elf-objcopy --weaken-symbol=ieee80211_raw_frame_sanity_check` command is applied to the stock `libnet80211.a` library. This converts the strong symbol to a weak symbol, allowing the linker to override it.
2. **Override Implementation**: A custom implementation of `ieee80211_raw_frame_sanity_check()` is provided that unconditionally returns `0` (success), bypassing all validation.
3. **Static Linking**: The patched library is statically linked into the firmware binary at compile time, ensuring the override persists across all WiFi operations.

This enables construction and transmission of arbitrary 802.11 frames including:
- **Deauthentication frames** (Management, Subtype 0x0C) with spoofed BSSID
- **Disassociation frames** (Management, Subtype 0x0A)
- **Beacon frames** (Management, Subtype 0x08) with arbitrary SSID IEs
- **Probe Response frames** (Management, Subtype 0x05)

### Promiscuous Mode Internals

When promiscuous mode is enabled via `esp_wifi_set_promiscuous(true)`, the ESP32's WiFi hardware is configured to pass all received 802.11 frames to the application layer, regardless of the destination MAC address. The callback registered with `esp_wifi_set_promiscuous_rx_cb()` receives:

- The raw 802.11 frame including the MAC header (Frame Control, Duration, Address fields, Sequence Control)
- Frame body (variable length depending on frame type)
- RSSI (Received Signal Strength Indicator) in dBm
- Channel number
- Frame length
- Timestamp (microsecond resolution)

TetraX parses these frames in real-time, extracting:
- BSSID, Source MAC, Destination MAC from the 802.11 header
- SSID from Beacon/Probe Response IE (Tag Number 0x00)
- Encryption type from RSN/WPA IE parsing
- Client-AP association pairs from Data frame headers

### BLE Advertisement Payload Engineering

The BLE spam module constructs raw advertising payloads that exploit the proximity notification systems of major mobile operating systems:

- **Apple Continuity Protocol**: Crafts manufacturer-specific data (Company ID: 0x004C) with Continuity message types (0x07 for Proximity Pairing, 0x10 for Nearby Action) that trigger persistent pairing dialogs on iOS devices.
- **Microsoft Swift Pair**: Generates advertising payloads with Microsoft vendor-specific data that trigger Windows "New device found" notifications.
- **Google Fast Pair**: Creates Model ID-based advertising payloads that Android's Nearby Service interprets as new peripheral discoveries.
- **Samsung BLE**: Crafts Samsung-specific manufacturer data payloads for Galaxy device notification triggering.

### NRF24L01+ Register-Level Manipulation

The CW jammer mode bypasses the NRF24's standard packet-oriented operation:

1. Power up the NRF24 (`PWR_UP=1` in CONFIG register)
2. Set to TX mode (`PRIM_RX=0`)
3. Enable continuous carrier wave (`CONT_WAVE=1, PLL_LOCK=1` in RF_SETUP register)
4. Set desired channel (0-125, corresponding to 2400-2525 MHz)
5. Assert CE HIGH to begin continuous transmission

This produces an unmodulated carrier that saturates a specific 1MHz channel in the ISM band.

---

## Project Structure

```
TetraX/
├── include/                    # Header files
│   ├── app_config.h            # Central pin definitions and firmware constants
│   ├── Deauther.h              # 802.11 deauthentication engine
│   ├── beacon_spam.h           # Beacon frame injection
│   ├── ble_spam.h              # BLE advertisement flood
│   ├── bt_analyzer.h           # Bluetooth Classic deep analyzer
│   ├── bt_jammer.h             # BT/BLE band disruption
│   ├── bt_remote.h             # BT HID keyboard emulation
│   ├── bt_spectrum.h           # BLE spectrum visualization
│   ├── btscan.h                # BLE device scanner
│   ├── channel_advisor.h       # WiFi channel recommendation
│   ├── coex_view.h             # WiFi/BT coexistence monitor
│   ├── dual_nrf_scope.h        # Dual NRF24 analysis
│   ├── hybrid_attack.h         # Multi-vector combined operations
│   ├── input_manager.h         # 5-button debounced input
│   ├── ir_manager.h            # IR TX/RX protocol handler
│   ├── app_ir_jammer.h         # IR carrier flood
│   ├── app_ir_receiver.h       # IR signal capture/decode
│   ├── app_ir_remote.h         # IR remote control emulator
│   ├── app_ir_sd.h             # IR signal SD card storage
│   ├── jammer.h                # NRF24 CW jammer
│   ├── menu_catalog.h          # Menu system definitions
│   ├── nrf_chat.h              # NRF24 radio chat
│   ├── nrf_link.h              # NRF24 data link
│   ├── nrf_helper.h            # NRF24 utility functions
│   ├── oled_mirror.h           # Web display mirror
│   ├── rf_heatmap.h            # NRF24 spectral heatmap
│   ├── rf_spectrum_analyzer.h  # Full-band spectrum analysis
│   ├── sd_manager.h            # SD card file operations
│   ├── slave_manager.h         # UART slave protocol
│   ├── spectrograph.h          # NRF24 spectrograph display
│   ├── total_jammer.h          # Multi-band saturation
│   ├── ui_theme.h              # OLED UI rendering engine
│   ├── web_server.h            # SoftAP web dashboard
│   ├── wifi_channel_scan.h     # Channel-by-channel scanner
│   ├── wifi_helper.h           # WiFi utility functions
│   ├── wifi_radar.h            # RSSI-based proximity radar
│   ├── animations.h            # Idle animation frame data
│   ├── flipper_assets.h        # Companion animation assets
│   ├── dolphinreactions.h      # Reactive animation triggers
│   ├── idle_mode.h             # Idle screen state machine
│   └── buzzer_manager.h        # Audio feedback control
│
├── src/                        # Source implementation files
│   ├── main.cpp                # Entry point, setup(), main loop
│   ├── Deauther.cpp            # 802.11 frame injection engine
│   ├── beacon_spam.cpp         # SSID beacon flood
│   ├── ble_spam.cpp            # BLE advertisement saturation
│   ├── bt_analyzer.cpp         # BT Classic deep analysis
│   ├── bt_jammer.cpp           # Bluetooth band disruption
│   ├── bt_remote.cpp           # HID keyboard injection
│   ├── bt_spectrum.cpp         # BLE spectrum rendering
│   ├── btscan.cpp              # BLE scan and discovery
│   ├── centinela.cpp           # Network watchdog monitor
│   ├── channel_advisor.cpp     # Channel recommendation logic
│   ├── coex_view.cpp           # Coexistence visualization
│   ├── dual_nrf_scope.cpp      # Dual-channel NRF analysis
│   ├── evil_portal.cpp         # Captive portal with DNS hijack
│   ├── gui_helper.cpp          # OLED animation and rendering
│   ├── hybrid_attack.cpp       # Combined multi-vector attack
│   ├── idle_mode.cpp           # Idle animation controller
│   ├── input_manager.cpp       # Button debounce and repeat
│   ├── ip_scanner.cpp          # ARP/ICMP host discovery
│   ├── ir_manager.cpp          # IR protocol engine
│   ├── app_ir_jammer.cpp       # IR carrier flood
│   ├── app_ir_remote.cpp       # IR remote emulation
│   ├── jammer.cpp              # NRF24 CW transmission
│   ├── main_ble.cpp            # BLE core initialization
│   ├── menu_catalog.cpp        # Menu tree definitions
│   ├── nrf_chat.cpp            # Radio messaging
│   ├── nrf_helper.cpp          # NRF24 SPI utilities
│   ├── nrf_link.cpp            # NRF24 data transfer
│   ├── oled_mirror.cpp         # Web-based display mirror
│   ├── packet_monitor.cpp      # Frame rate monitor
│   ├── rf_heatmap.cpp          # Spectral heatmap engine
│   ├── rf_spectrum_analyzer.cpp# Full-band RF analyzer
│   ├── sd_manager.cpp          # SD card operations
│   ├── slave.control.cpp       # Slave mode command handler
│   ├── slave_manager.cpp       # Slave protocol state machine
│   ├── spectrograph.cpp        # NRF24 spectrograph
│   ├── total_jammer.cpp        # Multi-band jammer
│   ├── ui_theme.cpp            # Theme rendering engine
│   ├── about_info.cpp          # System info display
│   ├── buzzer_manager.cpp      # Buzzer feedback
│   ├── wifi_channel_scan.cpp   # Per-channel WiFi scan
│   ├── wifi_radar.cpp          # RSSI proximity radar
│   ├── wifiscan.cpp            # Network enumeration
│   ├── dino_game.cpp           # Dino Runner game
│   ├── flappy_game.cpp         # Flappy Bird game
│   ├── snake_game.cpp          # Snake game
│   ├── pong_game.cpp           # Pong game
│   ├── space_game.cpp          # Space Invaders game
│   ├── games_menu.cpp          # Games submenu
│   └── patched_libnet80211.a.bin  # Patched WiFi library blob
│
├── platformio.ini              # Build configuration
├── weaken_libnet80211.py       # Library patching script
├── LICENSE                     # Project license
└── README.md                   # This file
```

---

## Version History

```
TetraX — Development Timeline
==============================

v1.0.0 [June 2026] ─── Initial Architecture
│
│   Core WiFi subsystem (Promiscuous mode, Deauther, Beacon Spam)
│   BLE spam module (Apple, Microsoft, Google, Samsung payloads)
│   NRF24L01+ integration (Jammer, Spectrum Analyzer)
│   SSD1306 OLED UI with 5-button navigation
│   IR Transceiver (Remote, Receiver, Jammer)
│   SD Card support (FAT32 file operations)
│   Evil Portal with DNS hijacking
│   BT Classic Analyzer and Scanner
│   Web Dashboard with OLED Mirror
│   Mini Games (Dino, Flappy, Snake, Pong, Space)
│   Sentinel network watchdog
│   IP Scanner with ARP/ICMP
│   Packet Monitor
│
├── v2.0.0 [Late June 2026] ─── RF Expansion
│   │
│   ├── RF Spectrum Analyzer (full 2.4GHz band sweep)
│   ├── RF Heatmap (temporal spectral mapping)
│   ├── Dual NRF Scope (simultaneous dual-channel)
│   ├── Spectrograph (waterfall display)
│   ├── NRF Chat (bidirectional radio messaging)
│   ├── NRF Link (high-speed RF data transfer)
│   ├── WiFi Radar (RSSI proximity visualization)
│   ├── Channel Advisor (optimal channel recommendation)
│   ├── WiFi/BT Coexistence View
│   └── Hybrid Attack mode (multi-vector coordination)
│
├── v3.0.0 [Early July 2026] ─── Master/Slave & External Hardware
│   │
│   ├── UART Master/Slave protocol (GPIO 16/14)
│   ├── Slave Manager state machine
│   ├── External USB Female port integration (GPIO 1/3)
│   ├── External BOOT button (GPIO 0 + GND)
│   ├── BT HID Keyboard emulation
│   ├── Flipper Zero / Bruce device integration
│   └── Power isolation circuitry for dual-battery operation
│
├── v4.0.0 [July 2026] ─── Production Release
│   │
│   ├── Complete English localization
│   ├── Animated idle screen with reactive companion
│   ├── Time-based constant-speed OLED animations
│   ├── Synchronous 5ms buzzer feedback (no blocking)
│   ├── IR boot suppression (GPIO 2 forced LOW on startup)
│   ├── Full rebranding to TetraX
│   ├── Optimized memory usage (26.1% RAM, 71.7% Flash)
│   └── Production-grade stability and polish
│
└── v5.0.0 [August 2026] ─── Master/Slave Dual-Band Ecosystem
    │
    ├── RAONE 5GHz Slave Link integration via Serial2 (RX2: GPIO 16, TX2: GPIO 17)
    ├── Hot-plug auto-detection via continuous background telemetry/heartbeat parser
    ├── Dual-Band Deauthentication coordination (2.4GHz on TetraX + 5GHz on RAONE)
    │   └── Note: Dual-band deauthentication implemented in firmware; pending field verification
    ├── Dual-Band Beacon Flooding across 2.4GHz and 5GHz channels simultaneously
    ├── Remote Keypad Forwarding (NAV/OK/BACK) from TetraX to RAONE co-processor
    ├── Standardized metric formatting across all screens (pkt/s, raw TX/Fail counts)
    ├── Channel Hopping indicator (CH: HOP) on live attack display
    ├── Dedicated USB-to-UART0 isolation (GPIO 1/3 for USB debugging, GPIO 16/17 for Slave link)
    └── Production stability updates and UI animations
```

---

## Building and Deployment

### Prerequisites

| Component | Requirement |
|---|---|
| Microcontroller | ESP32-WROOM-32 / ESP32-D0WD-V3 DevKit |
| Build System | PlatformIO Core CLI or VSCode Extension |
| Framework | Arduino (espressif32 @ 6.4.0) |
| Toolchain | xtensa-esp32 @ 8.4.0+2021r2-patch5 |
| NRF24L01+ | Optional (required for RF analysis features) |
| SD Card Module | Optional (required for file storage features) |
| IR LED + TSOP38238 | Optional (required for IR features) |
| SSD1306 128x64 OLED | Required (primary display) |

### Build Commands

```bash
# Compile firmware
pio run -e esp32dev

# Compile and flash via USB
pio run -e esp32dev -t upload

# Flash via external USB-to-TTL port
# Hold BOOT (GPIO 0), press RST, release BOOT, then:
pio run -e esp32dev -t upload --upload-port COM<X>

# Monitor serial output
pio device monitor -b 115200
```

### Memory Footprint (V4.0 Production)

```
RAM:   [===       ]  26.1%  (85,568 / 327,680 bytes)
Flash: [=======   ]  71.7%  (2,254,957 / 3,145,728 bytes)
```

---

## Legal Notice and Regulatory Compliance

### Indian Telecommunications Law

This device and its firmware are subject to the following Indian regulatory frameworks:

- **Indian Telegraph Act, 1885 (Section 3, 20, 25)**: Establishes that the Central Government holds exclusive privilege over establishing, maintaining, and working telegraphs. Unauthorized interception or interference with wireless telegraphy communications is a punishable offense under Sections 20 and 25, carrying imprisonment of up to three years and/or fines.

- **Indian Wireless Telegraphy Act, 1933 (Section 3, 6)**: Prohibits the possession, use, or dealing of any wireless telegraphy apparatus without a license granted by the Central Government. Section 6 prescribes penalties including imprisonment up to three years and equipment confiscation for unauthorized possession or use of wireless transmission equipment.

- **Information Technology Act, 2000 (Section 43, 65, 66, 66B, 70)**: Section 43 imposes civil liability for unauthorized access, data interception, or introducing contaminants into computer systems. Section 66 criminalizes computer-related offenses including unauthorized access and data manipulation with imprisonment up to three years and fines up to five lakh rupees. Section 66B addresses receiving stolen computer resources.

- **Information Technology (Intermediary Guidelines and Digital Media Ethics Code) Rules, 2021**: Establishes due diligence obligations for intermediaries and prohibits activities that threaten public order, the sovereignty and integrity of India, or national security.

- **TRAI (Telecom Regulatory Authority of India) Regulations**: The use of signal interference, jamming, or disruption equipment is strictly regulated. Only authorized government agencies may operate RF jamming equipment under specific conditions.

- **Department of Telecommunications (DoT) Guidelines**: All wireless equipment operating in the 2.4GHz ISM band must comply with ETSI standards as adopted by DoT, including maximum EIRP limits and duty cycle restrictions.

### International Regulatory Framework

- **ITU Radio Regulations**: The International Telecommunication Union governs global spectrum allocation. The 2.4GHz ISM band (2400-2500 MHz) is allocated for Industrial, Scientific, and Medical purposes on a secondary basis, subject to local licensing conditions.

- **IEEE 802.11 Standards Compliance**: Deauthentication frame injection operates outside the boundaries of normal 802.11 station behavior. Protected Management Frames (802.11w) were specifically designed to mitigate this attack vector.

### Usage Disclaimer

TetraX is designed and intended exclusively for:
- Licensed network administrators performing authorized security audits
- RF engineers conducting electromagnetic compatibility (EMC) testing
- Authorized security researchers operating within controlled laboratory environments
- Educational institutions demonstrating wireless protocol vulnerabilities in supervised settings

**The operator bears full legal responsibility for ensuring compliance with all applicable local, national, and international laws governing wireless transmission, signal interception, and network security testing.** Unauthorized use of the capabilities implemented in this firmware — including but not limited to network disruption, unauthorized access, wireless signal interference, or privacy violation — constitutes a criminal offense in most jurisdictions worldwide.

**The developer (mx_sourav) provides this firmware strictly as a research and educational tool and accepts no liability for misuse, illegal deployment, or damages arising from the operation of this device.**

---

## Dependencies

| Library | Version | Purpose |
|---|---|---|
| RF24 | 1.6.1 | NRF24L01+ transceiver driver |
| U8g2 | 2.36.18 | SSD1306 OLED graphics library |
| ESP32Ping | 1.7.0 | ICMP Echo implementation |
| ESPAsyncWebServer | 3.6.0 | Async HTTP server for web dashboard |
| AsyncTCP | 3.3.2 | TCP stack for async web server |
| ESP32 BLE Keyboard | 0.3.2 | BT HID keyboard emulation |
| IRremote | 4.7.1 | IR protocol encode/decode |
| WiFi | 2.0.0 | ESP32 WiFi stack |
| ESP32 BLE Arduino | 2.0.0 | BLE stack |
| FS / SPIFFS | 2.0.0 | Filesystem abstraction |
| DNSServer | 2.0.0 | DNS hijacking for Evil Portal |
| SD | 2.0.0 | SD card FAT32 driver |
| SPI | 2.0.0 | Hardware SPI bus |
| Preferences | 2.0.0 | NVS key-value storage |

---

<div align="center">

**TetraX** is developed and maintained by **mx_sourav**

*Built for research. Built for understanding. Built to push boundaries.*

</div>
