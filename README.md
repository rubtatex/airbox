# AirBox Control

A lightweight ESP32 relay control system with a modern web interface and comprehensive REST API. Perfect for controlling solenoid valves & pumps.

![Status](https://img.shields.io/badge/status-active-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-ESP32-orange)

## 📋 Overview

AirBox is a lean, efficient solution for controlling 4 relay outputs via an ESP32 microcontroller. It features:

- **4 Relay Outputs** - Control pumps, valves, and other devices independently if you need lol
- **Modern Web Interface** - Beautiful, responsive dashboard for configuration and control
- **REST API** - Full programmatic control via HTTP endpoints
- **WiFi Management** - Easy WiFi setup with automatic AP fallback
- **Firmware OTA Update** - Upload new firmware directly from the web interface
- **Real-time Status** - Live WiFi and relay status updates
- **Memory Optimized** - Lean codebase for limited ESP32 resources

## 🚀 Features

### Web Dashboard
- **WiFi Status Display** - Real-time connection status and signal strength
- **Quick Relay Control** - One-click ON/OFF for each relay from the dashboard
- **WiFi Configuration** - Connect to different networks without code changes
- **Firmware Upload** - Update the device firmware OTA with a .bin file
- **Modern Dark UI** - Beautiful, responsive interface with smooth animations

### Relay Management
- **Individual Control** - Toggle each relay independently via web or API
- **Batch Control** - Control multiple relays with a single API call
- **Real-time Status** - Query relay states instantly
- **Visual Feedback** - Web dashboard shows current relay states

### Configuration
- **WiFi Management** - Connect to WiFi with SSID and password
- **AP Mode Fallback** - Automatically enters Access Point if WiFi connection fails
- **Default SSID** - "AirBox" (password: "12345678") for AP mode
- **WiFi Reset** - Clear settings and return to AP mode
- **Persistent Storage** - WiFi credentials saved to preferences

### API Endpoints
Control your device programmatically via REST API:

#### GET Endpoints
- `GET /state` - Get current state of all 4 relays
  ```json
  { "in1": 0, "in2": 0, "in3": 1, "in4": 0 }
  ```

- `GET /wifi/status` - Get WiFi connection info
  ```json
  { "connected": 1, "ssid": "MyNetwork", "ip": "192.168.1.100", "rssi": -45 }
  ```

- `GET /relay/multi?relay=0,2&state=1,0` - Control multiple relays
  ```
  relay: comma-separated relay indices (0-3)
  state: comma-separated states (0=OFF, 1=ON)
  ```

#### POST Endpoints
- `POST /relay/set` - Control a single relay
  ```json
  { "relay": 0, "state": 1 }
  ```
  Returns: `{ "success": 1, "in1": 1, "in2": 0, "in3": 1, "in4": 0 }`

## 📦 Hardware Requirements

- **ESP32** Development Board (e.g., ESP32-DevKit-C)
- **4-Channel Relay Module** (5V or 3.3V compatible)
- **USB Cable** for programming and power

### Pin Configuration
```cpp
GPIO 33 → Relay 1 (IN1)
GPIO 25 → Relay 2 (IN2)
GPIO 26 → Relay 3 (IN3)
GPIO 27 → Relay 4 (IN4)
```

## 📦 Installation

### Option 1: Flash Prebuilt Firmware (Easiest)

1. **Download the latest firmware**
   - Go to [GitHub Releases](https://github.com/rubtatex/airbox/releases)
   - Download `firmware.bin` from the latest release

2. **Flash to ESP32**
   
   **Using esptool.py:**
   ```bash
   pip install esptool
   esptool.py --chip esp32 --port COM3 write_flash 0x10000 firmware.bin
   ```
   *Replace `COM3` with your ESP32's serial port*

   **Using ESP Flash Tool:**
   - Download [ESP32 Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)
   - Select `firmware.bin` at address `0x10000`
   - Click Start to flash

3. **Access the device**
   - Connect to WiFi "AirBox" (password: 12345678)
   - Open browser to `http://192.168.4.1`

### Option 2: Build from Source

**Prerequisites:**
- [PlatformIO](https://platformio.org/) with ESP32 support
- USB Driver for ESP32 (CH340 or CP2102)

**Steps:**

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/airbox.git
   cd airbox
   ```

2. **Connect ESP32** via USB

3. **Build and Upload**
   ```bash
   pio run -e esp32dev -t upload
   pio device monitor
   ```

## ⚙️ Configuration

### Customize Relay Pins
Edit `src/main.cpp`:
```cpp
#define RELAY_IN1 33
#define RELAY_IN2 25
#define RELAY_IN3 26
#define RELAY_IN4 27
```

### Customize AP Mode WiFi
Edit `src/main.cpp`:
```cpp
#define WIFI_SSID "AirBox"
#define WIFI_PASSWORD "12345678"
```

## 🐛 Troubleshooting

### Cannot upload to ESP32
- Check USB driver installation
- Verify COM port in PlatformIO
- Hold BOOT button during upload if needed

### WiFi not connecting
- Verify SSID and password are correct
- Ensure 2.4GHz band is enabled (5GHz not supported)
- Check WiFi signal strength
- Reset WiFi and reconfigure

### Web interface not loading
- Check ESP32 is powered and running
- Verify connected to correct WiFi network
- Try `http://192.168.4.1/` in AP mode
- Check serial output for errors

### Relays not responding
- Check GPIO pin connections
- Verify relay module power supply
- Confirm pins match code configuration
- Test with multimeter

### API not responding
- Verify network connectivity
- Check correct IP and port 80
- Use curl or Postman to test
- Check serial output

## 📊 Technical Specifications

- **Microcontroller:** ESP32 DevKit
- **Framework:** Arduino (PlatformIO)
- **WiFi:** 802.11 b/g/n (2.4GHz)
- **Relays:** 4 independent outputs
- **HTTP Port:** 80
- **JSON Library:** cJSON
- **Memory:** Optimized for ESP32

## 📝 License

MIT License - feel free to use in personal and commercial projects.

## 🤝 Contributing

Contributions welcome! Feel free to submit pull requests or open issues.

## 📧 Support

For issues or questions:
1. Check the Troubleshooting section above
2. Open an issue on GitHub
3. Review API documentation in the web interface

---

**Made for IoT enthusiasts and automation lovers** ❤️
