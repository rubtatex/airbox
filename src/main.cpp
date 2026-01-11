#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <preferences.h>
#include <Update.h>
#include "cJSON.h"

#define RELAY_IN1 33
#define RELAY_IN2 25
#define RELAY_IN3 26
#define RELAY_IN4 27

#define WIFI_SSID "AirBox"
#define WIFI_PASSWORD "12345678"

WebServer server(80);
Preferences preferences;

uint8_t relay_states[4] = {0, 0, 0, 0};
int relay_pins[4] = {RELAY_IN1, RELAY_IN2, RELAY_IN3, RELAY_IN4};
String wifi_ssid_current = "";
String wifi_ip_current = "";
int8_t wifi_rssi = -100;
uint8_t wifi_connected = 0;
cJSON *translations[2] = {NULL, NULL};
const char *lang_codes[2] = {"fr", "en"};

const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1.0">
    <title>AirBox Control</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            background: linear-gradient(135deg, #0f0f0f 0%, #1a1a2e 100%); 
            color: #e0e0e0; 
            padding: 20px; 
            min-height: 100vh;
        }
        .container { 
            max-width: 900px; 
            margin: 0 auto; 
        }
        header {
            text-align: center;
            margin-bottom: 40px;
            padding: 20px 0;
        }
        h1 { 
            color: #4db8ff; 
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 0 2px 10px rgba(77, 184, 255, 0.3);
        }
        .subtitle {
            color: #90caf9;
            font-size: 0.95em;
            margin-top: 5px;
        }
        .grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
            margin-bottom: 20px;
        }
        @media (max-width: 768px) {
            .grid { grid-template-columns: 1fr; }
        }
        .section { 
            background: rgba(45, 45, 45, 0.8);
            backdrop-filter: blur(10px);
            border-radius: 12px; 
            padding: 25px;
            border: 1px solid rgba(77, 184, 255, 0.2);
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
        }
        .section:hover {
            border-color: rgba(77, 184, 255, 0.4);
            box-shadow: 0 12px 48px rgba(77, 184, 255, 0.1);
        }
        .section h2 { 
            font-size: 1.3em; 
            color: #4db8ff; 
            margin-bottom: 15px;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .icon {
            width: 24px;
            height: 24px;
        }
        .form-group { 
            margin-bottom: 12px; 
        }
        label { 
            display: block; 
            margin-bottom: 6px; 
            color: #ddd; 
            font-weight: 600; 
            font-size: 0.9em;
        }
        input, select { 
            width: 100%; 
            padding: 11px; 
            border: 2px solid rgba(77, 184, 255, 0.3); 
            border-radius: 8px; 
            background: rgba(45, 45, 45, 0.6); 
            color: #e0e0e0; 
            font-size: 0.95em;
            transition: all 0.2s;
        }
        input:focus, select:focus { 
            outline: none; 
            border-color: #4db8ff;
            background: rgba(45, 45, 45, 0.9);
            box-shadow: 0 0 10px rgba(77, 184, 255, 0.2);
        }
        button { 
            width: 100%; 
            padding: 12px 20px;
            background: linear-gradient(135deg, #4db8ff 0%, #2d8bb8 100%); 
            color: white; 
            border: none; 
            border-radius: 8px; 
            font-weight: 600; 
            cursor: pointer; 
            font-size: 0.95em;
            transition: all 0.3s;
            box-shadow: 0 4px 15px rgba(77, 184, 255, 0.2);
        }
        button:hover { 
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(77, 184, 255, 0.4);
        }
        button:active { 
            transform: translateY(0);
        }
        .message { 
            padding: 12px 15px; 
            border-radius: 8px; 
            margin-top: 12px; 
            font-size: 0.9em; 
            display: none;
            border-left: 4px solid;
            animation: slideIn 0.3s ease;
        }
        @keyframes slideIn {
            from { transform: translateX(-20px); opacity: 0; }
            to { transform: translateX(0); opacity: 1; }
        }
        .message.success { 
            background: rgba(30, 70, 32, 0.8); 
            color: #81c784; 
            border-color: #4caf50;
            display: block;
        }
        .message.error { 
            background: rgba(74, 31, 31, 0.8); 
            color: #ef5350; 
            border-color: #ff6b6b;
            display: block;
        }
        .info-box { 
            background: rgba(31, 58, 90, 0.6);
            border-left: 4px solid #4db8ff;
            padding: 12px 15px; 
            border-radius: 8px; 
            color: #90caf9; 
            font-size: 0.85em;
            margin-top: 12px;
            line-height: 1.6;
        }
        .status-badge {
            display: inline-block;
            padding: 8px 15px;
            border-radius: 20px;
            font-size: 0.9em;
            font-weight: 600;
            margin-top: 12px;
        }
        .status-connected {
            background: rgba(30, 70, 32, 0.8);
            color: #4caf50;
            border: 1px solid #4caf50;
        }
        .status-disconnected {
            background: rgba(74, 31, 31, 0.8);
            color: #ff6b6b;
            border: 1px solid #ff6b6b;
        }
        .status-info {
            font-size: 0.85em;
            color: #aaa;
            margin-top: 8px;
        }
        .api-grid {
            display: grid;
            grid-template-columns: 1fr;
            gap: 12px;
        }
        .endpoint {
            background: rgba(60, 60, 60, 0.5);
            padding: 12px;
            border-radius: 6px;
            border-left: 3px solid;
            font-size: 0.85em;
            font-family: 'Courier New', monospace;
        }
        .endpoint.get {
            border-left-color: #4caf50;
        }
        .endpoint.post {
            border-left-color: #ff9800;
        }
        .method {
            display: inline-block;
            padding: 3px 8px;
            border-radius: 4px;
            font-weight: 600;
            font-size: 0.75em;
            margin-right: 8px;
        }
        .method.get {
            background: rgba(76, 175, 80, 0.2);
            color: #4caf50;
        }
        .method.post {
            background: rgba(255, 152, 0, 0.2);
            color: #ff9800;
        }
        .endpoint-desc {
            display: block;
            color: #90caf9;
            margin-top: 4px;
        }
        .full-width {
            grid-column: 1 / -1;
        }
        .button-group {
            display: flex;
            gap: 10px;
        }
        .button-group button {
            flex: 1;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>⚙️ AirBox Control</h1>
            <p class="subtitle">ESP32 Relay Management System</p>
        </header>

        <div class="grid">
            <!-- WiFi Status -->
            <div class="section">
                <h2>📡 WiFi Status</h2>
                <div id="wifi-status"></div>
            </div>

            <!-- Relay Control -->
            <div class="section">
                <h2>🔌 Quick Relay Control</h2>
                <div class="form-group">
                    <label for="relay-select">Select Relay</label>
                    <select id="relay-select">
                        <option value="0">Relay 1</option>
                        <option value="1">Relay 2</option>
                        <option value="2">Relay 3</option>
                        <option value="3">Relay 4</option>
                    </select>
                </div>
                <div class="button-group">
                    <button onclick="setRelay(1)" style="background: linear-gradient(135deg, #4caf50 0%, #388e3c 100%);">ON</button>
                    <button onclick="setRelay(0)" style="background: linear-gradient(135deg, #f44336 0%, #d32f2f 100%);">OFF</button>
                </div>
                <div id="relay-message" class="message"></div>
            </div>

            <!-- WiFi Configuration -->
            <div class="section full-width">
                <h2>🌐 WiFi Configuration</h2>
                <div class="form-group">
                    <label for="ssid">WiFi SSID</label>
                    <input type="text" id="ssid" placeholder="Enter network name">
                </div>
                <div class="form-group">
                    <label for="password">Password</label>
                    <input type="password" id="password" placeholder="Enter password">
                </div>
                <div class="button-group">
                    <button onclick="saveWiFi()" style="background: linear-gradient(135deg, #4db8ff 0%, #2d8bb8 100%);">Connect to WiFi</button>
                    <button onclick="resetWiFi()" style="background: linear-gradient(135deg, #ff6b6b 0%, #cc5555 100%);">Reset to AP Mode</button>
                </div>
                <div id="wifi-message" class="message"></div>
                <div class="info-box">WiFi changes will restart the device automatically.</div>
            </div>

            <!-- Firmware Upload -->
            <div class="section full-width">
                <h2>📦 Firmware Upload</h2>
                <div class="form-group">
                    <label for="firmware-file">Select firmware file (.bin)</label>
                    <input type="file" id="firmware-file" accept=".bin">
                </div>
                <button onclick="uploadFirmware()" style="background: linear-gradient(135deg, #ff9800 0%, #f57c00 100%);">Upload Firmware</button>
                <div id="firmware-message" class="message"></div>
                <div class="info-box">Upload a new firmware binary file to update the device. The device will restart after upload.</div>
            </div>

            <!-- API Documentation -->
            <div class="section full-width">
                <h2>📚 API Endpoints</h2>
                <div class="api-grid">
                    <div class="endpoint get">
                        <span class="method get">GET</span>
                        <strong>/state</strong>
                        <span class="endpoint-desc">Get current relay states</span>
                    </div>
                    <div class="endpoint get">
                        <span class="method get">GET</span>
                        <strong>/wifi/status</strong>
                        <span class="endpoint-desc">Get WiFi connection status and signal strength</span>
                    </div>
                    <div class="endpoint post">
                        <span class="method post">POST</span>
                        <strong>/relay/set</strong>
                        <span class="endpoint-desc">Control relay - JSON: {"relay": 0-3, "state": 0|1}</span>
                    </div>
                    <div class="endpoint get">
                        <span class="method get">GET</span>
                        <strong>/relay/multi?relay=0,2&state=1,0</strong>
                        <span class="endpoint-desc">Control multiple relays at once</span>
                    </div>
                    <div class="endpoint post">
                        <span class="method post">POST</span>
                        <strong>/wifi/config</strong>
                        <span class="endpoint-desc">Configure WiFi - JSON: {"ssid": "...", "password": "..."}</span>
                    </div>
                    <div class="endpoint post">
                        <span class="method post">POST</span>
                        <strong>/firmware/upload</strong>
                        <span class="endpoint-desc">Upload new firmware - multipart/form-data with 'firmware' field</span>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        function updateWiFiStatus() {
            fetch('/wifi/status')
                .then(r => r.json())
                .then(d => {
                    var statusHtml = '';
                    if (d.connected) {
                        statusHtml = '<div class="status-badge status-connected">✓ Connected</div>';
                        statusHtml += '<div class="status-info">Network: <strong>' + d.ssid + '</strong><br>IP: ' + d.ip + '<br>Signal: ' + d.rssi + ' dBm</div>';
                    } else {
                        statusHtml = '<div class="status-badge status-disconnected">⚠ AP Mode</div>';
                        statusHtml += '<div class="status-info">IP: 192.168.4.1<br>SSID: AirBox</div>';
                    }
                    document.getElementById('wifi-status').innerHTML = statusHtml;
                })
                .catch(e => {
                    document.getElementById('wifi-status').innerHTML = '<div class="status-badge status-disconnected">✗ Error</div>';
                });
        }

        function setRelay(state) {
            var relay = document.getElementById('relay-select').value;
            var msg = document.getElementById('relay-message');

            fetch('/relay/set', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ relay: parseInt(relay), state: state })
            })
                .then(r => r.json())
                .then(d => {
                    if (d.success) {
                        msg.className = 'message success';
                        msg.textContent = 'Relay ' + (parseInt(relay) + 1) + ' is now ' + (state ? 'ON' : 'OFF');
                    } else {
                        msg.className = 'message error';
                        msg.textContent = 'Error controlling relay';
                    }
                    setTimeout(() => msg.style.display = 'none', 3000);
                })
                .catch(e => {
                    msg.className = 'message error';
                    msg.textContent = 'Error: ' + e;
                    setTimeout(() => msg.style.display = 'none', 3000);
                });
        }

        function saveWiFi() {
            var ssid = document.getElementById('ssid').value;
            var pwd = document.getElementById('password').value;
            var msg = document.getElementById('wifi-message');

            if (!ssid || !pwd) {
                msg.className = 'message error';
                msg.textContent = 'SSID and password required';
                return;
            }

            fetch('/wifi/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ ssid: ssid, password: pwd })
            })
                .then(r => r.json())
                .then(d => {
                    if (d.success) {
                        msg.className = 'message success';
                        msg.textContent = 'Configuration saved. Device restarting...';
                        document.getElementById('ssid').value = '';
                        document.getElementById('password').value = '';
                    }
                })
                .catch(e => {
                    msg.className = 'message error';
                    msg.textContent = 'Error: ' + e;
                });
        }

        function resetWiFi() {
            if (confirm('Reset WiFi configuration and restart in AP mode?')) {
                fetch('/wifi/reset', { method: 'POST' })
                    .then(r => r.json())
                    .then(d => {
                        if (d.success) {
                            var msg = document.getElementById('wifi-message');
                            msg.className = 'message success';
                            msg.textContent = 'WiFi reset. Device restarting to AP mode...';
                        }
                    })
                    .catch(e => alert('Error: ' + e));
            }
        }

        function uploadFirmware() {
            var fileInput = document.getElementById('firmware-file');
            var file = fileInput.files[0];
            var msg = document.getElementById('firmware-message');

            if (!file) {
                msg.className = 'message error';
                msg.textContent = 'Please select a firmware file';
                return;
            }

            if (file.size === 0) {
                msg.className = 'message error';
                msg.textContent = 'File is empty';
                return;
            }

            msg.className = 'message';
            msg.textContent = 'Uploading firmware... Please wait';

            var formData = new FormData();
            formData.append('firmware', file);

            fetch('/firmware/upload', {
                method: 'POST',
                body: formData
            })
                .then(r => r.json())
                .then(d => {
                    if (d.success) {
                        msg.className = 'message success';
                        msg.textContent = 'Firmware uploaded successfully. Device is restarting...';
                        fileInput.value = '';
                    } else {
                        msg.className = 'message error';
                        msg.textContent = 'Error: ' + (d.message || 'Unknown error');
                    }
                })
                .catch(e => {
                    msg.className = 'message error';
                    msg.textContent = 'Upload failed: ' + e;
                });
        }

        // Initialize
        updateWiFiStatus();
        setInterval(updateWiFiStatus, 5000);
    </script>
</body>
</html>
)rawliteral";

// CORS helpers
void addCorsHeaders() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handle_options() {
    addCorsHeaders();
    server.send(204);
}

void register_options(const char* path) {
    server.on(path, HTTP_OPTIONS, handle_options);
}



void handle_root() {
    addCorsHeaders();
    server.send(200, "text/html", html_page);
}

void handle_state() {
    addCorsHeaders();
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "in1", relay_states[0]);
    cJSON_AddNumberToObject(root, "in2", relay_states[1]);
    cJSON_AddNumberToObject(root, "in3", relay_states[2]);
    cJSON_AddNumberToObject(root, "in4", relay_states[3]);
    
    char *json_str = cJSON_Print(root);
    server.send(200, "application/json", json_str);
    
    free(json_str);
    cJSON_Delete(root);
}

void handle_relay_multi() {
    addCorsHeaders();
    if (server.hasArg("relay") && server.hasArg("state")) {
        String relayStr = server.arg("relay");
        String stateStr = server.arg("state");
        
        char relayBuf[50], stateBuf[50];
        strncpy(relayBuf, relayStr.c_str(), sizeof(relayBuf) - 1);
        strncpy(stateBuf, stateStr.c_str(), sizeof(stateBuf) - 1);
        
        int relayCount = 0;
        char *token = strtok(relayBuf, ",");
        while (token && relayCount < 4) {
            int idx = atoi(token);
            token = strtok(NULL, ",");
            
            token = strtok(stateBuf, ",");
            if (token && idx >= 0 && idx <= 3) {
                int state = atoi(token);
                relay_states[idx] = state ? 1 : 0;
                digitalWrite(relay_pins[idx], !relay_states[idx]);
            }
            relayCount++;
            strcpy(stateBuf, "");
        }
        
        cJSON *root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "in1", relay_states[0]);
        cJSON_AddNumberToObject(root, "in2", relay_states[1]);
        cJSON_AddNumberToObject(root, "in3", relay_states[2]);
        cJSON_AddNumberToObject(root, "in4", relay_states[3]);
        
        char *json_str = cJSON_Print(root);
        server.send(200, "application/json", json_str);
        free(json_str);
        cJSON_Delete(root);
        return;
    }
    server.send(400, "application/json", "{\"error\":\"Invalid parameters\"}");
}

void handle_wifi_status() {
    addCorsHeaders();
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "connected", wifi_connected);
    cJSON_AddStringToObject(root, "ssid", wifi_ssid_current.c_str());
    cJSON_AddStringToObject(root, "ip", wifi_ip_current.c_str());
    cJSON_AddNumberToObject(root, "rssi", wifi_rssi);
    
    char *json_str = cJSON_Print(root);
    server.send(200, "application/json", json_str);
    
    free(json_str);
    cJSON_Delete(root);
}

void handle_relay_set() {
    addCorsHeaders();
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        cJSON *root = cJSON_Parse(body.c_str());
        
        if (root) {
            cJSON *relay_item = cJSON_GetObjectItem(root, "relay");
            cJSON *state_item = cJSON_GetObjectItem(root, "state");
            
            if (relay_item && state_item) {
                int relay = relay_item->valueint;
                uint8_t state = state_item->valueint ? 1 : 0;
                
                if (relay >= 0 && relay <= 3) {
                    relay_states[relay] = state;
                    digitalWrite(relay_pins[relay], !state);
                    
                    cJSON *response = cJSON_CreateObject();
                    cJSON_AddNumberToObject(response, "success", 1);
                    char *json_str = cJSON_Print(response);
                    server.send(200, "application/json", json_str);
                    free(json_str);
                    cJSON_Delete(response);
                    cJSON_Delete(root);
                    return;
                }
            }
            cJSON_Delete(root);
        }
    }
    server.send(400, "application/json", "{\"success\":0}");
}

void handle_wifi_config() {
    addCorsHeaders();
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        cJSON *root = cJSON_Parse(body.c_str());
        
        if (root) {
            cJSON *ssid_item = cJSON_GetObjectItem(root, "ssid");
            cJSON *password_item = cJSON_GetObjectItem(root, "password");
            
            if (ssid_item && password_item && ssid_item->valuestring && password_item->valuestring) {
                preferences.begin("wifi", false);
                preferences.putString("ssid", ssid_item->valuestring);
                preferences.putString("password", password_item->valuestring);
                preferences.end();
                
                cJSON *response = cJSON_CreateObject();
                cJSON_AddNumberToObject(response, "success", 1);
                char *json_str = cJSON_Print(response);
                server.send(200, "application/json", json_str);
                free(json_str);
                cJSON_Delete(response);
                cJSON_Delete(root);
                
                delay(2000);
                ESP.restart();
                return;
            }
            cJSON_Delete(root);
        }
    }
    server.send(400, "application/json", "{\"success\":0}");
}

void handle_wifi_reset() {
    addCorsHeaders();
    preferences.begin("wifi", false);
    preferences.clear();
    preferences.end();
    
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response, "success", 1);
    char *json_str = cJSON_Print(response);
    server.send(200, "application/json", json_str);
    free(json_str);
    cJSON_Delete(response);
    
    delay(2000);
    ESP.restart();
}

void handle_firmware_upload() {
    addCorsHeaders();
    
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("[OTA] Update start: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.println("[OTA] Update complete!");
            cJSON *response = cJSON_CreateObject();
            cJSON_AddNumberToObject(response, "success", 1);
            cJSON_AddStringToObject(response, "message", "Firmware updated successfully");
            char *json_str = cJSON_Print(response);
            server.send(200, "application/json", json_str);
            free(json_str);
            cJSON_Delete(response);
            
            delay(2000);
            ESP.restart();
        } else {
            Serial.println("[OTA] Update failed!");
            cJSON *response = cJSON_CreateObject();
            cJSON_AddNumberToObject(response, "success", 0);
            cJSON_AddStringToObject(response, "message", "Firmware update failed");
            char *json_str = cJSON_Print(response);
            server.send(500, "application/json", json_str);
            free(json_str);
            cJSON_Delete(response);
        }
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
        Update.printError(Serial);
        server.send(400, "application/json", "{\"success\":0,\"message\":\"Upload aborted\"}");
    }
}

void wifi_event_handler(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            wifi_connected = 1;
            Serial.println("[WiFi] Connected to WiFi network");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            wifi_connected = 0;
            Serial.println("[WiFi] Disconnected from WiFi");
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            wifi_ip_current = WiFi.localIP().toString();
            Serial.print("[WiFi] Got IP address: ");
            Serial.println(wifi_ip_current);
            break;
        default:
            break;
    }
}

void setup_wifi_sta(String ssid, String password);
void setup_wifi_ap();

void setup_wifi_sta(String ssid, String password) {
    Serial.print("[WiFi] Connecting to: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    wifi_ssid_current = ssid;
    
    for (int i = 0; i < 20; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            wifi_connected = 1;
            wifi_ip_current = WiFi.localIP().toString();
            Serial.println("[WiFi] Successfully connected!");
            Serial.print("[WiFi] IP address: ");
            Serial.println(wifi_ip_current);
            Serial.print("[WiFi] Signal strength: ");
            Serial.print(WiFi.RSSI());
            Serial.println(" dBm");
            return;
        }
        delay(500);
    }
    Serial.println("[WiFi] Connection timeout - switching to AP mode");
    setup_wifi_ap();
}

void setup_wifi_ap() {
    Serial.println("[WiFi] Starting Access Point mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    wifi_ssid_current = WIFI_SSID;
    wifi_ip_current = "192.168.4.1";
    Serial.print("[WiFi] AP SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("[WiFi] AP IP: ");
    Serial.println(wifi_ip_current);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n[AirBox] Starting...");
    
    for (int i = 0; i < 4; i++) {
        pinMode(relay_pins[i], OUTPUT);
        digitalWrite(relay_pins[i], HIGH);
    }
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }
    
    // Load relay names from preferences removed - keeping API but no storage
    
    WiFi.onEvent(wifi_event_handler);
    
    preferences.begin("wifi", true);
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();
    
    if (ssid.length() > 0 && password.length() > 0) {
        setup_wifi_sta(ssid, password);
    } else {
        setup_wifi_ap();
    }
    
    server.on("/", handle_root);
    server.on("/state", handle_state);
    server.on("/relay/multi", handle_relay_multi);
    server.on("/wifi/status", handle_wifi_status);
    server.on("/relay/set", HTTP_POST, handle_relay_set);
    server.on("/wifi/config", HTTP_POST, handle_wifi_config);
    server.on("/wifi/reset", HTTP_POST, handle_wifi_reset);
    server.on("/firmware/upload", HTTP_POST, handle_firmware_upload, handle_firmware_upload);

    register_options("/");
    register_options("/state");
    register_options("/relay/multi");
    register_options("/wifi/status");
    register_options("/relay/set");
    register_options("/wifi/config");
    register_options("/wifi/reset");
    register_options("/firmware/upload");
    
    server.begin();
}

void loop() {
    server.handleClient();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_rssi = WiFi.RSSI();
    }
    
    delay(10);
}
