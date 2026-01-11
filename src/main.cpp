#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <preferences.h>
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
    <title>AirBox Config</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { 
            font-family: Arial, sans-serif; 
            background: #1a1a1a; 
            color: #e0e0e0; 
            padding: 20px; 
            min-height: 100vh;
        }
        .container { 
            max-width: 600px; 
            margin: 0 auto; 
            background: #2d2d2d; 
            border-radius: 10px; 
            padding: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }
        h1 { 
            color: #fff; 
            margin-bottom: 20px; 
            text-align: center;
        }
        .section { 
            margin-bottom: 20px; 
            background: #3a3a3a; 
            padding: 15px; 
            border-radius: 8px; 
            border-left: 4px solid #4db8ff;
        }
        .section h2 { 
            font-size: 1em; 
            color: #4db8ff; 
            margin-bottom: 10px;
        }
        .form-group { 
            margin-bottom: 12px; 
        }
        label { 
            display: block; 
            margin-bottom: 5px; 
            color: #ddd; 
            font-weight: bold; 
            font-size: 0.9em;
        }
        input { 
            width: 100%; 
            padding: 10px; 
            border: 2px solid #444; 
            border-radius: 6px; 
            background: #2d2d2d; 
            color: #e0e0e0; 
            font-size: 0.95em;
        }
        input:focus { 
            outline: none; 
            border-color: #4db8ff;
        }
        button { 
            width: 100%; 
            padding: 12px; 
            background: linear-gradient(135deg, #4db8ff 0%, #2d8bb8 100%); 
            color: white; 
            border: none; 
            border-radius: 6px; 
            font-weight: bold; 
            cursor: pointer; 
            font-size: 0.95em;
            transition: transform 0.2s;
        }
        button:hover { 
            transform: translateY(-2px);
        }
        button:active { 
            transform: translateY(0);
        }
        .message { 
            padding: 10px; 
            border-radius: 6px; 
            margin-top: 10px; 
            font-size: 0.85em; 
            display: none;
        }
        .message.success { 
            background: #1e4620; 
            color: #4caf50; 
            border: 2px solid #4caf50; 
            display: block;
        }
        .message.error { 
            background: #4a1f1f; 
            color: #ff6b6b; 
            border: 2px solid #ff6b6b; 
            display: block;
        }
        .info-box { 
            background: #1f3a5a; 
            border-left: 4px solid #4db8ff;
            padding: 12px; 
            border-radius: 6px; 
            color: #90caf9; 
            font-size: 0.85em;
            margin-top: 10px;
            line-height: 1.5;
        }
        .status-badge {
            display: inline-block;
            padding: 6px 12px;
            border-radius: 20px;
            font-size: 0.9em;
            font-weight: bold;
            margin-top: 10px;
        }
        .status-connected {
            background: #1e4620;
            color: #4caf50;
        }
        .status-disconnected {
            background: #4a1f1f;
            color: #ff6b6b;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>AirBox Config</h1>
        
        <!-- WiFi Configuration -->
        <div class="section">
            <h2>WiFi Configuration</h2>
            <div id="wifi-status"></div>
            
            <div class="form-group" style="margin-top: 15px;">
                <label for="ssid">WiFi SSID</label>
                <input type="text" id="ssid" placeholder="Enter network name">
            </div>
            
            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" placeholder="Enter password">
            </div>
            
            <button onclick="saveWiFi()">Connect to WiFi</button>
            <div id="wifi-message" class="message"></div>
            <div class="info-box">The ESP32 will restart after saving WiFi configuration.</div>
        </div>
        
        <!-- Reset WiFi -->
        <div class="section">
            <h2>Reset WiFi</h2>
            <button onclick="resetWiFi()" style="background: linear-gradient(135deg, #ff6b6b 0%, #cc5555 100%);">Reset to AP Mode</button>
            <div class="info-box" style="margin-top: 10px;">This will erase WiFi settings and restart in Access Point mode (192.168.4.1).</div>
        </div>
        
        <!-- API Documentation -->
        <div class="section">
            <h2>API Endpoints</h2>
            <div style="font-size: 0.85em; color: #aaa; line-height: 1.6;">
                <strong>GET /state</strong> - Get relay states<br>
                <strong>GET /wifi/status</strong> - Get WiFi status<br>
                <strong>POST /relay/set</strong> - Control relay (JSON: {"relay": 0, "state": 1})<br>
                <strong>POST /wifi/config</strong> - Set WiFi (JSON: {"ssid": "...", "password": "..."})<br>
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
                        statusHtml = '<div class="status-badge status-connected">Connected to ' + d.ssid + '</div>';
                        statusHtml += '<div style="font-size:0.85em;color:#aaa;margin-top:8px;">IP: ' + d.ip + ' | Signal: ' + d.rssi + ' dBm</div>';
                    } else {
                        statusHtml = '<div class="status-badge status-disconnected">AP Mode (192.168.4.1)</div>';
                    }
                    document.getElementById('wifi-status').innerHTML = statusHtml;
                })
                .catch(e => console.log('Status error:', e));
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
                        msg.textContent = 'Configuration saved. Restarting...';
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
                        if (d.success) alert('Reset done. Restarting...');
                    })
                    .catch(e => alert('Error: ' + e));
            }
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

void handle_translations() {
    addCorsHeaders();
    server.send(404, "application/json", "{}");
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

void handle_relay_names() {
    addCorsHeaders();
    if (server.method() == HTTP_GET) {
        cJSON *root = cJSON_CreateObject();
        cJSON *names = cJSON_CreateArray();
        
        // Return empty or default names - relay names removed from ESP to save memory
        cJSON_AddItemToArray(names, cJSON_CreateString("Relay 1"));
        cJSON_AddItemToArray(names, cJSON_CreateString("Relay 2"));
        cJSON_AddItemToArray(names, cJSON_CreateString("Relay 3"));
        cJSON_AddItemToArray(names, cJSON_CreateString("Relay 4"));
        
        cJSON_AddItemToObject(root, "names", names);
        char *json_str = cJSON_Print(root);
        server.send(200, "application/json", json_str);
        free(json_str);
        cJSON_Delete(root);
        return;
    }
    
    server.send(400, "application/json", "{\"success\":0}");
}

void handle_relay_control() {
    addCorsHeaders();
    if (server.hasArg("relay") && server.hasArg("state")) {
        int relay = server.arg("relay").toInt();
        int state = server.arg("state").toInt();
        
        if (relay >= 0 && relay <= 3) {
            relay_states[relay] = state ? 1 : 0;
            digitalWrite(relay_pins[relay], !relay_states[relay]);
            
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
    }
    server.send(400, "application/json", "{\"error\":\"Invalid parameters\"}");
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
    server.on("/api/translations", handle_translations);
    server.on("/state", handle_state);
    server.on("/relay/names", handle_relay_names);
    server.on("/relay/control", handle_relay_control);
    server.on("/relay/multi", handle_relay_multi);
    server.on("/wifi/status", handle_wifi_status);
    server.on("/relay/set", HTTP_POST, handle_relay_set);
    server.on("/wifi/config", HTTP_POST, handle_wifi_config);
    server.on("/wifi/reset", HTTP_POST, handle_wifi_reset);

    register_options("/");
    register_options("/api/translations");
    register_options("/state");
    register_options("/relay/names");
    register_options("/relay/control");
    register_options("/relay/multi");
    register_options("/wifi/status");
    register_options("/relay/set");
    register_options("/wifi/config");
    register_options("/wifi/reset");
    
    server.begin();
}

void loop() {
    server.handleClient();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_rssi = WiFi.RSSI();
    }
    
    delay(10);
}
