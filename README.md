# AirBox Control

A lightweight ESP32 relay control system with minimal web interface and comprehensive REST API. Designed to offload UI complexity to a remote server while keeping the ESP32 lean and responsive.

![Status](https://img.shields.io/badge/status-active-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-ESP32-orange)

## 📋 Overview

AirBox is a lean solution for controlling relays and solenoid valves via an ESP32 microcontroller. It features:

- **4 Relay Outputs** - Control lights, pumps, valves, and other devices
- **Minimal Web Interface** - Lightweight configuration page for WiFi setup
- **REST API** - Full control via HTTP endpoints for remote servers
- **WiFi Management** - Easy WiFi configuration with AP fallback mode
- **Memory Efficient** - Optimized for ESP32 with limited resources
- **Remote UI Ready** - Design your own control interface on a separate server

## 🚀 Features

### Minimal Web Interface
- **Configuration Page** - WiFi setup and status display
- **API Documentation** - Quick reference for REST endpoints
- **Lightweight** - Minimal CSS and JavaScript to save memory

### Relay Management
- **Individual Control** - Toggle each relay independently
- **Real-time Status** - Query relay states via API
- **Visual Feedback** - Web interface shows current status
- **Persistent Storage** - No relay names stored (reduce memory usage)

### Configuration
- **WiFi Management** - Connect to WiFi networks with SSID/Password
- **AP Mode Fallback** - Automatically enters Access Point mode if connection fails
- **Signal Strength** - Real-time WiFi status indicator
- **Easy Reset** - Reset WiFi configuration and restart in AP mode

### API Endpoints (REST)
All relay control is done via these APIs, perfect for remote servers:

#### GET Endpoints
- `GET /state` - Get current state of all relays
- `GET /relay/control?relay=0&state=1` - Control single relay (relay: 0-3, state: 0 or 1)
- `GET /relay/multi?relay=0,2&state=1,0` - Control multiple relays
- `GET /relay/names` - Get default relay names
- `GET /wifi/status` - Get WiFi connection status

#### POST Endpoints
- `POST /relay/set` - Set relay state via JSON: `{"relay": 0, "state": 1}`
- `POST /wifi/config` - Configure WiFi: `{"ssid": "MyWiFi", "password": "pass123"}`
- `POST /wifi/reset` - Reset WiFi configuration and restart in AP mode

## 📦 Hardware Requirements

- **ESP32** Development Board (e.g., ESP32-DevKit-C)
- **4-Channel Relay Module** (5V or 3.3V compatible)
- **USB Cable** for programming and power
- **Optional:** WiFi antenna for better range

### Pin Configuration
```
GPIO 33 → Relay IN1
GPIO 25 → Relay IN2
GPIO 26 → Relay IN3 (Normally Open Valve)
GPIO 27 → Relay IN4 (Normally Closed Valve)
```

## 🔧 Installation

### Prerequisites
- [PlatformIO](https://platformio.org/) with ESP32 support
- USB Driver for ESP32 (CH340 or CP2102)

### Setup Steps

1. **Clone or Download** this repository
   ```bash
   git clone https://github.com/yourusername/airbox-control.git
   cd airbox-control
   ```

2. **Structure du projet:**
   ```
   airbox-control/
   ├── src/
   │   └── main.cpp
   ├── data/
   │   ├── translations_fr.json
   │   └── translations_en.json
   ├── platformio.ini
   └── README.md
   ```

3. **Connect ESP32** via USB

4. **Build and Upload:**
   ```bash
   # Upload firmware only (no SPIFFS needed anymore)
   pio run -e esp32dev -t upload
   
   # Monitor serial output
   pio device monitor
   ```

5. **Access the Interface**
   - **AP Mode:** Connect to `AirBox` WiFi network (password: `12345678`)
   - **URL:** `http://192.168.4.1/`
   - **After WiFi config:** Access via device IP on your home network or use API from remote server

## 📱 Usage

### Web Interface

The web interface is minimal and serves two purposes:

#### Status Display
- View current WiFi connection status
- See connected network name and signal strength
- Check device IP address

#### Configuration
- **WiFi Settings:** Configure your home network
  - Enter SSID and password
  - Click "Connect to WiFi" to save and restart
- **Reset WiFi:** Return to AP mode by clicking "Reset to AP Mode"

#### Available Community Interfaces

| Interface | Repository | Description |
|-----------|------------|-------------|
| AirBox Remote Control | [rubtatex/airbox-remote-control](https://github.com/rubtatex/airbox-remote-control) | Complete web interface for remote control |

### Controlling Relays via API

Since relay names are not stored on the device, use your remote server to manage the UI. The ESP32 provides all the APIs needed:

**Python Example:**
```python
import requests

BASE_URL = "http://192.168.4.1"

# Get relay status
response = requests.get(f"{BASE_URL}/state")
print(response.json())  # {'in1': 0, 'in2': 1, 'in3': 0, 'in4': 0}

# Control relay
response = requests.post(f"{BASE_URL}/relay/set", 
    json={"relay": 0, "state": 1})
print(response.json())  # {'success': 1}

# Get WiFi status
response = requests.get(f"{BASE_URL}/wifi/status")
print(response.json())
```

### Default Settings
- **WiFi SSID (AP Mode):** `AirBox`
- **WiFi Password (AP Mode):** `12345678`
- **Default IP (AP Mode):** `192.168.4.1`
- **HTTP Port:** 80

## 🌐 Remote UI Architecture

Your remote server can:
- Host a rich web interface for multiple users
- Manage relay scheduling and automation
- Store historical data and logs
- Provide authentication and permissions
- Integrate with other systems (MQTT, home automation, etc.)


## 🔌 API Usage Examples

**Control Relay via GET:**
```bash
# Turn ON relay 0
curl "http://192.168.4.1/relay/control?relay=0&state=1"

# Turn OFF relay 0
curl "http://192.168.4.1/relay/control?relay=0&state=0"

# Open valve (relay 2)
curl "http://192.168.4.1/relay/control?relay=2&state=1"
```

**Control Multiple Relays:**
```bash
curl "http://192.168.4.1/relay/multi?relay=0,2&state=1,0"
```

**Get Relay Status:**
```bash
curl "http://192.168.4.1/state"
```

**Rename Relays:**
```bash
curl -X POST http://192.168.4.1/relay/names \
  -H "Content-Type: application/json" \
  -d '{"names":["Pump","Light","Valve1","Valve2"]}'
```

**Configure WiFi:**
```bash
curl -X POST http://192.168.4.1/wifi/config \
  -H "Content-Type: application/json" \
  -d '{"ssid":"MyWiFi","password":"mypassword"}'
```

**Check WiFi Status:**
```bash
curl "http://192.168.4.1/wifi/status"
```

## 📤 Uploading Updates

### Uploading Firmware
```bash
pio run -e esp32dev -t upload
```

### Erase and Reinstall
```bash
pio run -e esp32dev -t erase
pio run -e esp32dev -t upload
```

## ⚙️ Configuration

### Relay Pin Mapping
Edit `main.cpp` to change GPIO pins:
```cpp
#define RELAY_IN1 33
#define RELAY_IN2 25
#define RELAY_IN3 26
#define RELAY_IN4 27
```

### WiFi Settings
Edit `main.cpp` to change AP mode defaults:
```cpp
#define WIFI_SSID "AirBox"
#define WIFI_PASSWORD "12345678"
```

### Server Port
HTTP server runs on port 80 (standard). To change, modify in `main.cpp`:
```cpp
WebServer server(80);  // Change 80 to desired port
```

## 🐛 Troubleshooting

### Cannot upload to ESP32
- Check USB driver installation
- Verify COM port in PlatformIO
- Hold BOOT button during upload if needed
- Disconnect and reconnect USB cable

### WiFi not connecting
- Verify SSID and password are correct
- Ensure 2.4GHz band is enabled (5GHz not supported)
- Check WiFi signal strength
- Reset WiFi configuration and reconfigure

### Web interface not loading
- Verify ESP32 is powered and running
- Check correct WiFi network is connected
- Try accessing `http://192.168.4.1/` (AP mode IP)
- Check serial monitor for errors

### Relays not responding
- Check GPIO pin connections
- Verify relay module power supply
- Confirm relay control pins match code
- Check serial output for debug messages
- Test relay module directly with multimeter

### API not responding
- Verify ESP32 has network connectivity
- Check firewall rules on ESP32's WiFi network
- Ensure correct IP address and port (80)
- Use curl or Postman to test endpoints
- Check serial monitor for request logs

## 🔒 Security Notes

- Change default WiFi password in production
- Consider adding authentication for public networks
- WiFi credentials stored in NVS (non-volatile storage)
- Relay names stored in device memory
- Use HTTPS for sensitive deployments (not included)

## 📊 Technical Specifications

- **Microcontroller:** ESP32 
- **Framework:** Arduino with PlatformIO
- **WiFi Standard:** 802.11 b/g/n (2.4GHz only)
- **Max Relays:** 4 independent outputs
- **HTTP Server:** Built-in ESP32 WebServer (port 80)
- **Storage:** Minimal (no SPIFFS needed for translations)
- **JSON Library:** cJSON (for API responses)
- **Memory:** Optimized for ESP32 constraints

## 📝 License

This project is licensed under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## 🎯 Future Enhancements

- [ ] MQTT support for smart home integration
- [ ] Remote server example implementation
- [ ] Web authentication for remote UI
- [ ] OTA firmware updates

## 📧 Support

For issues, questions, or suggestions:
1. Check the Troubleshooting section
2. Open an issue on GitHub
3. Review API documentation in the web interface or this README

---

**Made with ❤️ for IoT enthusiasts and automation lovers.**

Built to be lean on ESP32, powerful on your server.
