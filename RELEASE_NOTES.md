# 📦 Release Notes

## v1.0.0 — Initial Release

**Release Date:** June 26, 2026

### ✨ New Features

#### IR Tools
- **IR Jammer** — Added 38 kHz carrier flood for IR channel testing
- **IR Remote** — Universal TV remote with multi-brand support:
  - Samsung, LG, Sony, Panasonic, and more
  - 8 commands per brand: Power, Vol+, Vol-, Ch+, Ch-, Mute, Home, Back
- **IR Receiver** — Capture, decode, and replay IR signals
- **IR from SD Card** — Load and replay IR signal recordings from microSD

#### AC Remote *(Experimental)*
- Initial AC remote control support added
- ⚠️ **Not stable yet** — still under active development and testing

#### SD Card Support
- Integrated microSD card reader on shared VSPI bus (CS: GPIO 13)
- Store and load IR signal recordings
- Persistent log storage for captured data

#### WiFi Toolkit
- WiFi Scanner, Radar, Channel Scanner, Packet Monitor
- Centinela passive watchdog mode
- IP Scanner with parallel ping sweeps
- Web Dashboard (SPA) served from the ESP32

#### RF Spectrum (nRF24L01+)
- RF Spectrum Analyzer and Heatmap
- Channel Advisor for cleanest channel recommendations
- NRF Link, NRF Chat, Dual NRF Scope
- BT/WiFi Coexistence viewer

#### Bluetooth
- BT Scanner, BT Analyzer, BT Spectrum visualization

#### SPA Web Dashboard
- Full Single-Page Application dashboard
- Real-time device status and control
- Accessible at `192.168.4.1` — will be hosted online for demo/documentation

#### Other
- 5 arcade mini-games (Dino, Flappy, Snake, Space, Pong)
- Slave device control via ESP-NOW
- Log viewer for captured data
- Custom themed OLED UI (128×64)
- Pre-built binaries included in `binarios/`

---

### 🔮 Coming Soon
- Stable AC remote support
- Companion app
- More IR brand profiles
- Online hosted SPA dashboard
