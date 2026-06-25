# ⚡ TetraX

> One of the most compact, bare-minimum-feathers cyber pen testing tool built on ESP32.

[![License: MIT](https://img.shields.io/badge/License-MIT-cyan.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32-blue.svg)](#hardware)
[![Version](https://img.shields.io/badge/Version-v1.0.0-green.svg)](RELEASE_NOTES.md)

---

## 📖 Overview

**TetraX** is a pocket-sized, all-in-one cybersecurity research tool powered by the ESP32 microcontroller. It packs WiFi analysis, Bluetooth reconnaissance, RF spectrum tools, IR transceiver capabilities, and a web-based SPA dashboard into a single compact device — perfect for security researchers, pen testers, and hardware enthusiasts.

---

## ✨ Features

### 📡 WiFi Toolkit
- **WiFi Scanner** — Scan and enumerate nearby access points with signal strength, channel, and encryption info
- **WiFi Radar** — Real-time visual radar display of surrounding wireless networks
- **Channel Scanner** — Per-channel activity analysis and congestion mapping
- **Packet Monitor** — Live 802.11 packet capture and traffic visualization
- **Centinela Mode** — Passive watchdog monitoring for network changes
- **IP Scanner** — Discover active hosts on a connected network with parallel ping sweeps
- **Web Dashboard** — Full SPA control panel served from the ESP32's built-in access point

### 📻 RF Tools (nRF24L01+)
- **RF Spectrum Analyzer** — Sweep 2.4 GHz band and visualize channel usage
- **RF Heatmap** — Color-coded heatmap of RF energy across channels
- **Channel Advisor** — Recommends the cleanest channel based on spectrum data
- **NRF Link** — Point-to-point data link between two nRF24 modules
- **NRF Chat** — Encrypted text messaging over nRF24
- **BT/WiFi Coexistence View** — Visualize interference between Bluetooth and WiFi
- **Dual NRF Scope** — Simultaneous monitoring with two nRF24 modules

### 🔵 Bluetooth
- **BT Scanner** — Discover classic Bluetooth devices in range
- **BT Analyzer** — Deep-dive analysis of detected Bluetooth devices
- **BT Spectrum** — Bluetooth frequency hopping visualization

### 🔴 IR Tools
- **IR Remote** — Universal TV remote supporting multiple brands:
  - Samsung, LG, Sony, Panasonic, and more
  - Commands: Power, Vol+/-, Ch+/-, Mute, Home, Back
- **AC Remote** — Air conditioner remote control *(⚠️ Not stable yet — experimental)*
- **IR Receiver** — Capture and decode incoming IR signals (protocol, address, command)
- **IR Jammer** — 38 kHz carrier flood for IR channel testing
- **IR from SD Card** — Load and replay IR signal recordings from microSD

### 💾 SD Card Support
- Load/save IR signal recordings
- Persistent storage for captured data and logs
- Shared VSPI bus with nRF24 modules

### 🎮 Built-in Games
- Arcade mini-games collection: Dino, Flappy, Snake, Space, Pong

### 🖥️ SPA Web Dashboard
- Single-Page Application served directly from the ESP32
- Real-time device status, network stats, and tool control
- Accessible via browser at `192.168.4.1` when connected to the device AP
- **Will be hosted online for documentation and demo purposes**

### ⚙️ System
- Slave device control over ESP-NOW
- Log viewer for captured data
- About screen with device info and firmware version
- OLED display (128×64 SSD1306) with custom themed UI

---

## 🔧 Hardware

| Component | Description |
|-----------|-------------|
| **MCU** | ESP32 DevKit V1 |
| **Display** | 0.96″ SSD1306 OLED (I2C — SDA:21, SCL:22) |
| **RF Module** | 2× nRF24L01+ (VSPI — CE:5/16, CSN:17/4) |
| **IR TX** | IR LED on GPIO 2 |
| **IR RX** | IR Receiver on GPIO 35 |
| **SD Card** | MicroSD module (VSPI — CS:13) |
| **Buttons** | 5 tactile buttons (UP:26, DOWN:33, OK:32, BACK:25, AUX:27) |
| **Power** | TP4056 LiPo charger + step-up converter |

---

## 🚀 Getting Started

### Prerequisites
- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- ESP32 DevKit V1 board
- USB cable for flashing

### Build & Flash
```bash
# Clone the repository
git clone https://github.com/mxsourav/TetraX.git
cd TetraX

# Build and upload via PlatformIO
pio run --target upload

# Monitor serial output
pio device monitor --baud 115200
```

### Pre-built Binaries
Pre-compiled firmware binaries are available in the [`binarios/`](binarios/) directory for direct flashing without building from source.

---

## 📁 Project Structure

```
TetraX/
├── src/                  # C++ source files (all modules)
├── include/              # Header files
├── companion_app/        # Companion application (WIP)
├── binarios/             # Pre-built firmware binaries
├── img/                  # Component images & diagrams
├── index.html            # SPA web dashboard
├── style.css             # Dashboard styles
├── script.js             # Dashboard logic
├── manifest.json         # PWA manifest
├── platformio.ini        # PlatformIO build configuration
└── RELEASE_NOTES.md      # Version changelog
```

---

## 📦 Dependencies

| Library | Version |
|---------|---------|
| RF24 | ^1.4.7 |
| U8g2 | ^2.35.9 |
| ESP32Ping | ^1.7 |
| ESPAsyncWebServer | latest |
| AsyncTCP | latest |
| ESP32 BLE Keyboard | ^0.3.2 |
| IRremote | latest |

---

## 👤 Author

**mxsourav**

---

## 📄 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## ⚠️ Disclaimer

This tool is intended for **authorized security research and educational purposes only**. Always obtain proper authorization before testing any network or device. The author is not responsible for any misuse.
