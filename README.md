# AirBox Control

A powerful web-based control interface for ESP32 relay management with multilingual support (French & English), WiFi configuration, and comprehensive REST API.

**Languages:** 🇬🇧 English • 🇫🇷 Français

![Status](https://img.shields.io/badge/status-active-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-ESP32-orange)

## 📋 Overview

AirBox is a complete solution for controlling relays and solenoid valves via an ESP32 microcontroller. It features:

- **4 Relay Outputs** - Control lights, pumps, valves, and other devices
- **Web Interface** - Beautiful, responsive UI accessible from any browser
- **Multilingual** - Full support for French and English (easily extensible)
- **Customizable Relay Names** - Rename each relay to your needs
- **WiFi Management** - Easy WiFi configuration with AP fallback mode
- **REST API** - Full control via HTTP endpoints
- **Real-time Updates** - Live status monitoring
- **Signal Strength Indicator** - Visual WiFi signal quality

## 🚀 Features

### Control Interface
- **Relays Tab** - Control 4 independent relays with ON/OFF or Open/Close buttons
- **Configuration Tab** - Manage WiFi, rename relays, and configure the system
- **API Tab** - Documentation with dynamic IP addresses and copy-to-clipboard URLs

### Relay Management
- **Individual Control** - Toggle each relay independently
- **Custom Naming** - Rename relays from default (Libre, Pompe, Valve NF, Valve NO)
- **Real-time Status** - See relay states in real-time (updates every 2 seconds)
- **Visual Feedback** - Color-coded state indicators (Green=ON, Gray=OFF)
- **Persistent Storage** - Relay names are saved to memory

### Configuration
- **Relay Names** - Customize names for each relay and save them
- **WiFi Management** - Connect to WiFi networks with SSID/Password
- **AP Mode Fallback** - Automatically enters Access Point mode if connection fails
- **Signal Strength** - Real-time WiFi signal indicator (📶 🌐 ❌)
- **Easy Reset** - Reset WiFi configuration and restart in AP mode

### API Endpoints

#### GET Endpoints
- `GET /state` - Get current state of all relays
- `GET /relay/control?relay=0&state=1` - Control single relay (relay: 0-3, state: 0 or 1)
- `GET /relay/multi?relay=0,2&state=1,0` - Control multiple relays
- `GET /relay/names` - Get current relay names
- `GET /wifi/status` - Get WiFi connection status
- `GET /api/translations?lang=en` - Get UI translations

#### POST Endpoints
- `POST /relay/set` - Set relay state via JSON: `{"relay": 0, "state": 1}`
- `POST /relay/names` - Set relay names: `{"names": ["Name1", "Name2", "Name3", "Name4"]}`
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
   # Upload firmware
   pio run -e esp32dev -t upload
   
   # Upload translations (SPIFFS)
   pio run -e esp32dev -t uploadfs
   
   # Monitor serial output
   pio device monitor
   ```

5. **Access the Interface**
   - **AP Mode:** Connect to `AirBox` WiFi network (password: `12345678`)
   - **URL:** `http://192.168.4.1/`
   - **After WiFi config:** Access via device IP on your home network

## 📱 Usage

### Web Interface

#### Relays Tab
- Click **ON/OFF** buttons to control relays
- Click **Open/Close** buttons for valve control
- Real-time status updates automatically

#### Configuration Tab
- **Rename Relays:** Customize names for each relay (IN1, IN2, IN3, IN4)
  - Enter new names and click "Save Relay Names"
  - Names are persistent and will be saved to the device memory
- **WiFi Settings:** Configure your home network
  - Enter SSID and password
  - Click "Save and Restart" to connect
- **Reset WiFi:** Return to AP mode by clicking "Reset WiFi Config"

#### API Tab
- View all available API endpoints
- Click URLs to copy to clipboard
- IP address updates dynamically

### Language Selection
Click the flag buttons (🇫🇷 / 🇬🇧) to switch UI language. Selection is saved locally in your browser.

### Default Settings
- **WiFi SSID:** `AirBox`
- **WiFi Password:** `12345678`
- **Default IP (AP Mode):** `192.168.4.1`
- **Default Language:** English

## 🌐 Translations

The interface includes translations for French and English. Translation files are stored in JSON format in the `data/` directory.

### Modifying Translations

1. Edit `data/translations_fr.json` or `data/translations_en.json`
2. Upload to the device:
   ```bash
   pio run -e esp32dev -t uploadfs
   ```
3. Refresh the web interface in your browser

### Adding New Languages

1. Create new translation file: `data/translations_xx.json`
2. Add language code to `main.cpp`:
   ```cpp
   const char *lang_codes[3] = {"fr", "en", "xx"};
   cJSON *translations[3] = {NULL, NULL, NULL};
   ```
3. Update the language selector in the HTML
4. Rebuild and upload both firmware and SPIFFS

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

### Uploading Firmware Only
```bash
pio run -e esp32dev -t upload
```

### Uploading Translations Only
```bash
pio run -e esp32dev -t uploadfs
```

### Uploading Both
```bash
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs
```

### Erase Everything
```bash
pio run -e esp32dev -t erase
pio run -e esp32dev -t upload
pio run -e esp32dev -t uploadfs
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

### Translations not showing
- Ensure `data/` folder contains JSON files
- Run `pio run -e esp32dev -t uploadfs`
- Check browser console (F12) for errors
- Verify JSON files are valid (use jsonlint.com)

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
- Verify SPIFFS files are uploaded

### Relays not responding
- Check GPIO pin connections
- Verify relay module power supply
- Confirm relay control pins match code
- Check serial output for debug messages
- Test relay module directly with multimeter

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
- **Storage:** SPIFFS for translations and settings
- **JSON Library:** cJSON
- **Languages:** French, English (extensible)

## 📝 License

This project is licensed under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## 🎯 Future Enhancements

- [ ] MQTT support for smart home integration
- [ ] Scheduling/programs functionality
- [ ] HTTPS support
- [ ] Web authentication
- [ ] Additional languages
- [ ] OTA firmware updates
- [ ] Mobile app

## 📧 Support

For issues, questions, or suggestions:
1. Check the Troubleshooting section
2. Open an issue on GitHub
3. Review API documentation in the web interface

---

**Made with ❤️ for IoT enthusiasts and automation lovers.**

For more information, visit the repository or check the inline code documentation.
