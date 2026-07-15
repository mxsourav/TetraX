# TetraX: Advanced RF Diagnostic & Network Auditing Subsystem

**Developer: mx_sourav**

TetraX is an advanced, multi-protocol RF diagnostic and security auditing platform designed for deep 802.11, Bluetooth Low Energy (BLE), and Sub-GHz infrastructure analysis. Built on the ESP32 microcontroller, TetraX leverages low-level hardware abstraction to manipulate network stacks far beyond standard operational boundaries, providing enterprise-grade packet injection, spectral analysis, and hardware coexistence diagnostics.

This repository contains the full source code for the TetraX firmware, enabling hardware-level manipulation of the ESP32's RF frontend.

## Core Technical Architecture

TetraX operates by bypassing the standard ESP-IDF Wi-Fi/Bluetooth stack restrictions to interface directly with the physical (PHY) and media access control (MAC) layers. The architecture is modular, operating across three primary domains:

### 1. 802.11 Promiscuous Mode & Frame Injection
Standard network interfaces discard frames not explicitly destined for their MAC address. TetraX utilizes the ESP32's `esp_wifi_set_promiscuous` API to drop the MAC layer filter, capturing all IEEE 802.11 traffic within the 2.4GHz band. 

- **Packet Sniffing & Parsing**: By hooking into the RX callback (`esp_wifi_set_promiscuous_rx_cb`), TetraX parses raw 802.11 frames (Management, Control, and Data frames). It extracts BSSID, RSSI, channel data, and encrypted payload signatures directly from the air, allowing passive mapping of hidden network topologies without association.
- **Raw Frame Injection (802.11w Bypass)**: The firmware utilizes a patched `libnet80211.a` to bypass Espressif's standard sanity checks on raw frame injection. This allows the construction and transmission of arbitrary 802.11 management frames.
  - *Deauthentication/Disassociation*: TetraX constructs highly targeted IEEE 802.11 Class 3 management frames with spoofed sender MAC addresses to forcibly sever client associations, enabling handshake capture and network stability testing.
  - *Beacon Spamming*: By crafting custom 802.11 Beacon frames containing Information Elements (IEs) specifying arbitrary SSIDs, TetraX saturates the surrounding RF environment with phantom access points, stress-testing the parsing logic of local wireless clients.

### 2. BLE Link Layer Manipulation
The Bluetooth Low Energy stack is stripped down to allow direct manipulation of advertising packets. 

- **Advertisement Saturation**: TetraX constructs malformed or highly repetitive BLE GAP (Generic Access Profile) advertising payloads. By injecting crafted payloads simulating Apple Continuity, Microsoft Swift Pair, or Google Fast Pair, the device forces nearby BLE controllers into continuous parsing loops, evaluating the resilience of modern smartphone OS background processing.
- **Spectrum Saturation**: TetraX rapidly cycles through the primary BLE advertising channels (37, 38, 39), maximizing spatial and temporal RF footprint to audit receiver desensitization.

### 3. Sub-GHz and RF Coexistence (NRF24)
TetraX integrates native support for the NRF24L01 transceiver module via SPI, allowing parallel 2.4GHz operations that intentionally evade standard 802.11/BLE collision avoidance protocols.

- **Continuous Wave (CW) Transmission**: By manipulating the NRF24 hardware registers directly (bypassing packet handling), TetraX forces the PLL into a continuous unmodulated carrier wave transmission state. This physically blocks specific 1MHz channels in the 2.4GHz ISM band, providing a benchmark for WiFi/BT coexistence and interference recovery algorithms.
- **Hardware Master/Slave Synchronization**: TetraX exposes raw hardware UART on GPIO 1 and 3, allowing it to act as a headless slave module coordinated by external hardware.

## Building and Deployment

TetraX relies on PlatformIO and requires a specific, statically linked library patch to enable raw frame injection. 

### Prerequisites
- ESP32 Development Board (WROOM-32 or WROVER based)
- PlatformIO Core CLI or VSCode Extension
- NRF24L01 module (optional, for Sub-GHz/Hardware Jamming routines)

### Compilation
The build environment forces the `framework-arduinoespressif32` core into a specific state. Do not update the core version beyond the locked dependency in `platformio.ini`, as changes to the proprietary Espressif blobs will break the injection patches.

```bash
pio run -e esp32dev -t upload
```

## Legal and Ethical Use

TetraX is engineered strictly for network administrators, RF engineers, and authorized security researchers. The low-level injection and interference capabilities implemented in this firmware are highly destructive to localized network infrastructure. 

It is the responsibility of the operator to ensure that this tool is deployed exclusively in controlled laboratory environments or on networks where explicit, legally binding authorization has been granted. Unauthorized use of 802.11 deauthentication, BLE spamming, or RF carrier wave interference may violate federal telecommunications laws (e.g., FCC regulations). 

**Developer:** mx_sourav
