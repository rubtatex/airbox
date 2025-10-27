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
String relay_names[4] = {"Libre", "Pompe", "Valve NF", "Valve NO"};

cJSON *translations[2] = {NULL, NULL};
const char *lang_codes[2] = {"fr", "en"};

const char* html_page = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width,initial-scale=1.0,viewport-fit=cover">
    <title>AirBox Control</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        html, body { width: 100%; height: 100%; overflow-x: hidden; }
        body { 
            font-family: Arial, sans-serif; 
            background: #1a1a1a; 
            min-height: 100vh; 
            display: flex; 
            justify-content: center; 
            align-items: flex-start; 
            padding: 10px; 
            padding-top: 20px; 
            color: #e0e0e0; 
        }
        .container { 
            background: #2d2d2d; 
            border-radius: 15px; 
            box-shadow: 0 20px 60px rgba(0,0,0,0.5); 
            padding: 15px; 
            width: 100%; 
            max-width: 600px; 
            max-height: calc(100vh - 40px); 
            overflow-y: auto; 
        }
        .header { 
            display: flex; 
            justify-content: space-between; 
            align-items: center; 
            margin-bottom: 20px; 
        }
        h1 { 
            color: #fff; 
            font-size: 1.5em; 
            margin: 0; 
        }
        .lang-selector {
            display: flex;
            gap: 8px;
        }
        .lang-btn {
            padding: 6px 12px;
            border: 2px solid #444;
            background: #3a3a3a;
            color: #e0e0e0;
            border-radius: 5px;
            cursor: pointer;
            font-size: 1.2em;
            font-weight: bold;
        }
        .lang-btn.active {
            border-color: #4db8ff;
            color: #4db8ff;
        }
        .tabs { 
            display: flex; 
            gap: 5px; 
            margin-bottom: 15px; 
            border-bottom: 2px solid #444; 
            position: sticky; 
            top: 0; 
            background: #2d2d2d; 
            padding: 0 0 10px 0; 
            z-index: 10; 
            overflow-x: auto; 
        }
        .tab-btn { 
            padding: 8px 12px; 
            border: none; 
            background: none; 
            cursor: pointer; 
            font-weight: bold; 
            color: #999; 
            border-bottom: 3px solid transparent; 
            white-space: nowrap; 
            font-size: 0.85em; 
        }
        .tab-btn.active { 
            color: #4db8ff; 
            border-bottom-color: #4db8ff; 
        }
        .tab-content { 
            display: none; 
        }
        .tab-content.active { 
            display: block; 
        }
        .relay-grid { 
            display: grid; 
            grid-template-columns: 1fr 1fr; 
            gap: 10px; 
            margin-bottom: 10px; 
        }
        @media (max-width:480px) {
            .relay-grid { grid-template-columns: 1fr; gap: 8px; }
            .header { flex-direction: column; gap: 10px; text-align: center; }
        }
        .relay-card { 
            background: #3a3a3a; 
            border-radius: 10px; 
            padding: 12px; 
            text-align: center; 
            border: 2px solid #444; 
        }
        .relay-card div:first-child { 
            font-size: 0.75em; 
            color: #888; 
            text-transform: uppercase; 
            letter-spacing: 0.5px; 
            margin-bottom: 3px; 
        }
        .relay-card div:nth-child(2) { 
            font-size: 1em; 
            font-weight: bold; 
            color: #ddd; 
            margin-bottom: 5px; 
        }
        .relay-state { 
            width: 45px; 
            height: 45px; 
            border-radius: 50%; 
            margin: 5px auto; 
            display: flex; 
            align-items: center; 
            justify-content: center; 
            font-size: 0.7em; 
            font-weight: bold; 
            color: white; 
            text-align: center; 
            padding: 2px; 
        }
        .relay-state.on { 
            background: linear-gradient(135deg, #28a745 0%, #20c997 100%); 
        }
        .relay-state.off { 
            background: linear-gradient(135deg, #555 0%, #333 100%); 
        }
        .button-group { 
            display: flex; 
            gap: 6px; 
            margin-top: 8px; 
        }
        button { 
            flex: 1; 
            padding: 6px; 
            border: none; 
            border-radius: 5px; 
            font-weight: bold; 
            cursor: pointer; 
            color: white; 
            font-size: 0.8em; 
            min-height: 32px; 
        }
        .btn-on { 
            background: linear-gradient(135deg, #28a745 0%, #20c997 100%); 
        }
        .btn-off { 
            background: linear-gradient(135deg, #666 0%, #444 100%); 
        }
        .btn-on:active, .btn-off:active { 
            transform: scale(0.95); 
        }
        .form-group { 
            margin-bottom: 12px; 
        }
        label { 
            display: block; 
            margin-bottom: 5px; 
            color: #ddd; 
            font-weight: bold; 
            font-size: 0.85em; 
        }
        input, select { 
            width: 100%; 
            padding: 10px; 
            border: 2px solid #444; 
            border-radius: 6px; 
            font-size: 1em; 
            background: #3a3a3a; 
            color: #e0e0e0; 
        }
        input:focus, select:focus { 
            outline: none; 
            border-color: #4db8ff; 
            box-shadow: 0 0 0 3px rgba(77, 184, 255, 0.2); 
        }
        .btn-save { 
            width: 100%; 
            padding: 10px; 
            background: linear-gradient(135deg, #4db8ff 0%, #2d8bb8 100%); 
            color: white; 
            border: none; 
            border-radius: 6px; 
            font-weight: bold; 
            cursor: pointer; 
            font-size: 0.95em; 
            min-height: 40px; 
        }
        .btn-reset { 
            width: 100%; 
            padding: 10px; 
            background: linear-gradient(135deg, #ff6b6b 0%, #cc5555 100%); 
            color: white; 
            border: none; 
            border-radius: 6px; 
            font-weight: bold; 
            cursor: pointer; 
            font-size: 0.95em; 
            min-height: 40px; 
            margin-top: 8px; 
        }
        .message { 
            padding: 10px; 
            border-radius: 6px; 
            margin-bottom: 12px; 
            display: none; 
            font-size: 0.85em; 
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
        .info { 
            background: #1f3a5a; 
            border: 2px solid #4db8ff; 
            border-radius: 6px; 
            padding: 10px; 
            margin-top: 12px; 
            color: #90caf9; 
            font-size: 0.8em; 
            line-height: 1.3; 
        }
        .wifi-status-box { 
            background: #3a3a3a; 
            padding: 12px; 
            border-radius: 8px; 
            margin-bottom: 15px; 
            border-left: 4px solid #4db8ff; 
        }
        .wifi-status-label { 
            font-size: 0.85em; 
            color: #aaa; 
            margin-bottom: 5px; 
        }
        .wifi-status-value { 
            font-size: 1.1em; 
            font-weight: bold; 
            color: #4db8ff; 
            display: flex; 
            align-items: center; 
            gap: 8px; 
        }
        .wifi-icon { 
            font-size: 1.3em; 
        }
        .wifi-connected { 
            color: #4caf50; 
        }
        .wifi-disconnected { 
            color: #ff6b6b; 
        }
        .wifi-info-line { 
            font-size: 0.85em; 
            color: #aaa; 
            margin-top: 6px; 
            display: flex; 
            align-items: center; 
            gap: 8px; 
        }
        .api-section { 
            margin-bottom: 15px; 
            background: #3a3a3a; 
            padding: 12px; 
            border-radius: 8px; 
            border-left: 4px solid #4db8ff; 
        }
        .api-title { 
            font-weight: bold; 
            color: #4db8ff; 
            margin-bottom: 8px; 
            font-size: 0.9em; 
        }
        .api-url { 
            background: #2d2d2d; 
            padding: 8px; 
            border-radius: 4px; 
            font-family: monospace; 
            font-size: 0.75em; 
            color: #90caf9; 
            word-break: break-all; 
            margin-bottom: 5px; 
            border: 1px solid #444; 
            cursor: pointer;
            transition: all 0.2s;
        }
        .api-url:hover {
            background: #333;
            border-color: #4db8ff;
        }
        .api-desc { 
            font-size: 0.8em; 
            color: #aaa; 
            margin-top: 5px; 
        }
        code { 
            background: #2d2d2d; 
            padding: 2px 6px; 
            border-radius: 3px; 
            color: #90caf9; 
            font-family: monospace; 
            font-size: 0.85em; 
        }
        .relay-config-box {
            background: #3a3a3a;
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 12px;
            border-left: 4px solid #4db8ff;
        }
        .relay-config-label {
            font-size: 0.85em;
            color: #aaa;
            margin-bottom: 5px;
            font-weight: bold;
        }
        .relay-config-input {
            width: 100%;
            padding: 8px;
            border: 2px solid #444;
            border-radius: 6px;
            background: #2d2d2d;
            color: #e0e0e0;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>AirBox</h1>
            <div class="lang-selector">
                <button class="lang-btn" onclick="switchLang('fr')">🇫🇷</button>
                <button class="lang-btn active" onclick="switchLang('en')">🇬🇧</button>
            </div>
        </div>
        
        <div class="tabs">
            <button class="tab-btn active" onclick="switchTab(0)" data-key="tab_relays">Relays</button>
            <button class="tab-btn" onclick="switchTab(1)" data-key="tab_config">Configuration</button>
            <button class="tab-btn" onclick="switchTab(2)" data-key="tab_api">API</button>
        </div>
        
        <!-- TAB 0: RELAYS -->
        <div class="tab-content active" id="tab0">
            <div class="relay-grid" id="relays-grid">Loading...</div>
        </div>
        
        <!-- TAB 1: CONFIGURATION -->
        <div class="tab-content" id="tab1">
            <div class="wifi-status-box">
                <div class="wifi-status-label" data-key="wifi_status">WiFi Status</div>
                <div class="wifi-status-value">
                    <span class="wifi-icon" id="wifi-icon">📶</span>
                    <span id="wifi-status-text">Loading...</span>
                </div>
                <div id="wifi-ssid-display" class="wifi-info-line"></div>
                <div id="wifi-ip-display" class="wifi-info-line"></div>
                <div id="wifi-signal-display" class="wifi-info-line"></div>
            </div>
            
            <hr style="border:none;border-top:1px solid #444;margin:15px 0">
            
            <div id="config-relays"></div>
            
            <button class="btn-save" onclick="saveRelayNames()" data-key="btn_save_names">Save Relay Names</button>
            
            <hr style="border:none;border-top:1px solid #444;margin:15px 0">
            
            <div style="margin-bottom:15px">
                <button class="btn-reset" onclick="resetWiFi()" data-key="btn_reset_wifi">Reset WiFi Config</button>
                <div class="info" data-key="info_reset">This will erase the WiFi config and restart in AP mode.</div>
            </div>
            
            <div class="message" id="message"></div>
            
            <div class="form-group">
                <label data-key="label_ssid">WiFi SSID</label>
                <input type="text" id="ssid" placeholder="">
            </div>
            
            <div class="form-group">
                <label data-key="label_password">Password</label>
                <input type="password" id="password" placeholder="">
            </div>
            
            <button class="btn-save" onclick="saveWiFi()" data-key="btn_save_wifi">Save and Restart</button>
            <div class="info" data-key="info_wifi">The ESP32 will connect to the configured network and restart.</div>
        </div>
        
        <!-- TAB 2: API -->
        <div class="tab-content" id="tab2">
            <div class="api-section">
                <div class="api-title">GET /state</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-state">http://192.168.4.1/state</span></div>
                <div class="api-desc" data-key="api_state">Get the status of all relays</div>
            </div>
            
            <div class="api-section">
                <div class="api-title">GET /relay/control</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-control">http://192.168.4.1/relay/control?relay=0&state=1</span></div>
                <div class="api-desc" data-key="api_control">Control a relay via GET. relay: 0-3, state: 0 or 1</div>
            </div>
            
            <div class="api-section">
                <div class="api-title">GET /relay/multi</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-multi">http://192.168.4.1/relay/multi?relay=0,2&state=1,0</span></div>
                <div class="api-desc" data-key="api_multi">Control multiple relays. relay: indices (0-3), state: values (0 or 1)</div>
            </div>
            
            <div class="api-section">
                <div class="api-title">GET /wifi/status</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-wifi-status">http://192.168.4.1/wifi/status</span></div>
                <div class="api-desc" data-key="api_wifi_status">Returns WiFi status (ssid, ip, rssi, connected)</div>
            </div>
            
            <div class="api-section">
                <div class="api-title">GET /relay/names</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-relay-names">http://192.168.4.1/relay/names</span></div>
                <div class="api-desc" data-key="api_relay_names_get">Get current relay names</div>
            </div>
            
            <div class="api-section">
                <div class="api-title">POST /relay/set</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-relay-set">http://192.168.4.1/relay/set</span></div>
                <div class="api-desc" data-key="api_relay_set">JSON: <code>{"relay": 0, "state": 1}</code></div>
            </div>
            
            <div class="api-section">
                <div class="api-title">POST /relay/names</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-relay-names-post">http://192.168.4.1/relay/names</span></div>
                <div class="api-desc" data-key="api_relay_names_post">JSON: <code>{"names": ["Name1", "Name2", "Name3", "Name4"]}</code></div>
            </div>
            
            <div class="api-section">
                <div class="api-title">POST /wifi/config</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-wifi-config">http://192.168.4.1/wifi/config</span></div>
                <div class="api-desc" data-key="api_wifi_config">JSON: <code>{"ssid": "MyWiFi", "password": "pass123"}</code></div>
            </div>
            
            <div class="api-section">
                <div class="api-title">POST /relay/names</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-relay-names">http://192.168.4.1/relay/names</span></div>
                <div class="api-desc" data-key="api_relay_names">JSON: <code>{"names": ["Name1", "Name2", "Name3", "Name4"]}</code></div>
            </div>
            
            <div class="api-section">
                <div class="api-title">POST /wifi/reset</div>
                <div class="api-url" onclick="copyToClipboard(this)"><span id="url-wifi-reset">http://192.168.4.1/wifi/reset</span></div>
                <div class="api-desc" data-key="api_wifi_reset">Reset WiFi configuration</div>
            </div>
        </div>
    </div>

    <script>
        var currentIP = window.location.hostname;
        var currentLang = localStorage.getItem('airboxLang') || 'en';
        var translations = {};
        var relayNames = ["Libre", "Pompe", "Valve NF", "Valve NO"];

        function loadTranslations(lang) {
            fetch('/api/translations?lang=' + lang)
                .then(r => r.json())
                .then(data => {
                    translations = data;
                    updateUI();
                })
                .catch(e => console.log('Translation error:', e));
        }

        function t(key) {
            if (translations && translations[key]) {
                return translations[key];
            }
            return key;
        }

        function updateUI() {
            document.querySelectorAll('[data-key]').forEach(el => {
                const key = el.getAttribute('data-key');
                const text = t(key);
                if (el.tagName === 'INPUT') {
                    el.placeholder = text;
                } else {
                    el.textContent = text;
                }
            });
        }

        function switchLang(lang) {
            currentLang = lang;
            localStorage.setItem('airboxLang', lang);
            document.querySelectorAll('.lang-btn').forEach(b => b.classList.remove('active'));
            event.target.classList.add('active');
            loadTranslations(lang);
        }

        function loadRelayNames() {
            fetch('/relay/names')
                .then(r => r.json())
                .then(data => {
                    relayNames = data.names || relayNames;
                    renderRelayConfig();
                })
                .catch(e => console.log(e));
        }

        function renderRelayConfig() {
            var html = '';
            var labels = ['IN1', 'IN2', 'IN3', 'IN4'];
            
            for (var i = 0; i < 4; i++) {
                html += '<div class="relay-config-box">';
                html += '<div class="relay-config-label">' + labels[i] + '</div>';
                html += '<input type="text" id="relay-name-' + i + '" class="relay-config-input" value="' + relayNames[i] + '" placeholder="Relay ' + i + ' name">';
                html += '</div>';
            }
            
            document.getElementById('config-relays').innerHTML = html;
        }

        function saveRelayNames() {
            var names = [];
            for (var i = 0; i < 4; i++) {
                names.push(document.getElementById('relay-name-' + i).value);
            }
            
            fetch('/relay/names', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ names: names })
            })
                .then(r => r.json())
                .then(d => {
                    if (d.success) {
                        relayNames = names;
                        loadState();
                    }
                })
                .catch(e => console.log(e));
        }

        function updateIPInAPI() {
            var protocol = window.location.protocol.replace(':', '');
            var baseURL = protocol + '://' + currentIP;
            
            document.getElementById('url-state').textContent = baseURL + '/state';
            document.getElementById('url-control').textContent = baseURL + '/relay/control?relay=0&state=1';
            document.getElementById('url-multi').textContent = baseURL + '/relay/multi?relay=0,2&state=1,0';
            document.getElementById('url-wifi-status').textContent = baseURL + '/wifi/status';
            document.getElementById('url-relay-set').textContent = baseURL + '/relay/set';
            document.getElementById('url-wifi-config').textContent = baseURL + '/wifi/config';
            document.getElementById('url-relay-names').textContent = baseURL + '/relay/names';
            document.getElementById('url-relay-names-post').textContent = baseURL + '/relay/names';
            document.getElementById('url-wifi-reset').textContent = baseURL + '/wifi/reset';
        }

        function copyToClipboard(element) {
            var text = element.querySelector('span').textContent;
            navigator.clipboard.writeText(text).then(() => {
                var original = element.textContent;
                element.textContent = '✓ ' + t('copied');
                setTimeout(() => { element.textContent = original; }, 2000);
            });
        }

        function getWiFiIcon(rssi) {
            if (rssi >= -50) return '▁▂▃▄▅'; // Excellent
            if (rssi >= -60) return '▁▂▃▄'; // Very good
            if (rssi >= -70) return '▁▂▃'; // Good
            if (rssi >= -80) return '▁▂'; // Fair
            if (rssi >= -90) return '▁'; // Weak
            return '▅'; // No signal / offline
        }

        function switchTab(n) {
            var tabs = document.getElementsByClassName('tab-content');
            var btns = document.getElementsByClassName('tab-btn');
            for (var i = 0; i < tabs.length; i++) tabs[i].classList.remove('active');
            for (var i = 0; i < btns.length; i++) btns[i].classList.remove('active');
            document.getElementById('tab' + n).classList.add('active');
            btns[n].classList.add('active');
            if (n === 0) loadState();
            if (n === 1) { loadRelayNames(); updateWiFiStatus(); }
            if (n === 2) updateIPInAPI();
        }

        function loadState() {
            fetch('/state')
                .then(r => r.json())
                .then(d => updateUIRelays(d))
                .catch(e => {
                    console.log(e);
                    setTimeout(loadState, 2000);
                });
        }

        function updateUIRelays(d) {
            var html = '';
            var names = ['IN1', 'IN2', 'IN3', 'IN4'];

            for (var i = 0; i < 4; i++) {
                var key = 'in' + (i + 1);
                var state = d[key] ? 1 : 0;
                var cls = state ? 'on' : 'off';
                var txt, btn1Txt_key, btn1Val, btn2Txt_key, btn2Val;

                if (i === 2) {
                    txt = state ? 'OUV' : 'FRM';
                    btn1Txt_key = 'btn_open';
                    btn1Val = 1;
                    btn2Txt_key = 'btn_close';
                    btn2Val = 0;
                } else if (i === 3) {
                    txt = state ? 'FRM' : 'OUV';
                    btn1Txt_key = 'btn_open';
                    btn1Val = 0;
                    btn2Txt_key = 'btn_close';
                    btn2Val = 1;
                } else {
                    txt = state ? 'ON' : 'OFF';
                    btn1Txt_key = 'btn_on';
                    btn1Val = 1;
                    btn2Txt_key = 'btn_off';
                    btn2Val = 0;
                }

                var desc = relayNames[i];
                var btn1Txt = t(btn1Txt_key);
                var btn2Txt = t(btn2Txt_key);

                html += '<div class="relay-card"><div>' + names[i] + '</div><div>' + desc + '</div><div class="relay-state ' + cls + '">' + txt + '</div><div class="button-group"><button class="btn-on" onclick="setRelay(' + i + ',' + btn1Val + ')">' + btn1Txt + '</button><button class="btn-off" onclick="setRelay(' + i + ',' + btn2Val + ')">' + btn2Txt + '</button></div></div>';
            }
            document.getElementById('relays-grid').innerHTML = html;
        }

        function setRelay(r, s) {
            fetch('/relay/set', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ relay: r, state: s })
            })
                .then(res => res.json())
                .then(d => { if (d.success) loadState(); })
                .catch(e => console.log(e));
        }

        function updateWiFiStatus() {
            fetch('/wifi/status')
                .then(r => r.json())
                .then(d => {
                    var statusIcon = document.getElementById('wifi-icon');
                    var statusText = document.getElementById('wifi-status-text');
                    var ssidDisplay = document.getElementById('wifi-ssid-display');
                    var ipDisplay = document.getElementById('wifi-ip-display');
                    var signalDisplay = document.getElementById('wifi-signal-display');

                    if (d.connected) {
                        statusIcon.textContent = getWiFiIcon(d.rssi);
                        statusText.textContent = t('wifi_connected');
                        statusText.className = 'wifi-connected';
                        ssidDisplay.innerHTML = '<span style="color:#888">' + t('wifi_network') + ':</span> ' + d.ssid;
                        ipDisplay.innerHTML = '<span style="color:#888">' + t('wifi_ip') + ':</span> ' + d.ip;
                        signalDisplay.innerHTML = '<span style="color:#888">' + t('wifi_signal') + ':</span> ' + d.rssi + ' dBm';
                    } else {
                        statusIcon.textContent = '📡';
                        statusText.textContent = t('wifi_disconnected');
                        statusText.className = 'wifi-disconnected';
                        ssidDisplay.innerHTML = '<span style="color:#888">' + t('wifi_mode') + ':</span> AP (AirBox)';
                        ipDisplay.innerHTML = '<span style="color:#888">' + t('wifi_ip') + ':</span> 192.168.4.1';
                        signalDisplay.innerHTML = '';
                    }
                })
                .catch(e => console.log('WiFi status error:', e));
        }

        function saveWiFi() {
            var ssid = document.getElementById('ssid').value;
            var pwd = document.getElementById('password').value;
            var msg = document.getElementById('message');

            if (!ssid || !pwd) {
                msg.className = 'message error';
                msg.textContent = t('error_required');
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
                        msg.textContent = t('msg_config_saved');
                        setTimeout(() => alert(t('msg_reconnect')), 2000);
                    }
                })
                .catch(e => {
                    msg.className = 'message error';
                    msg.textContent = t('error_save');
                });
        }

        function resetWiFi() {
            if (confirm(t('confirm_reset'))) {
                fetch('/wifi/reset', { method: 'POST' })
                    .then(r => r.json())
                    .then(d => {
                        if (d.success) {
                            alert(t('msg_reset_done'));
                            setTimeout(() => location.reload(), 1000);
                        }
                    })
                    .catch(e => alert(t('error_reset')));
            }
        }

        // Initialize
        loadTranslations(currentLang);
        updateIPInAPI();
        loadRelayNames();
        loadState();
        setInterval(loadState, 2000);
        setInterval(updateWiFiStatus, 5000);
    </script>
</body>
</html>
)rawliteral";

void load_translations(void) {
    for (int i = 0; i < 2; i++) {
        char filepath[64];
        snprintf(filepath, sizeof(filepath), "/spiffs/translations_%s.json", lang_codes[i]);
        
        FILE *f = fopen(filepath, "r");
        if (!f) {
            continue;
        }
        
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        if (fsize <= 0 || fsize > 10000) {
            fclose(f);
            continue;
        }
        
        char *content = (char*)malloc(fsize + 1);
        if (!content) {
            fclose(f);
            continue;
        }
        
        fread(content, 1, fsize, f);
        fclose(f);
        content[fsize] = 0;
        
        translations[i] = cJSON_Parse(content);
        free(content);
    }
}

void handle_root() {
    server.send(200, "text/html", html_page);
}

void handle_translations() {
    if (server.hasArg("lang")) {
        String lang = server.arg("lang");
        
        int lang_idx = -1;
        for (int i = 0; i < 2; i++) {
            if (lang == lang_codes[i] && translations[i]) {
                lang_idx = i;
                break;
            }
        }
        
        if (lang_idx >= 0 && translations[lang_idx]) {
            char *json_str = cJSON_Print(translations[lang_idx]);
            server.send(200, "application/json", json_str);
            free(json_str);
            return;
        }
    }
    server.send(404, "application/json", "{}");
}

void handle_state() {
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
    if (server.method() == HTTP_POST && server.hasArg("plain")) {
        String body = server.arg("plain");
        cJSON *root = cJSON_Parse(body.c_str());
        
        if (root) {
            cJSON *names_array = cJSON_GetObjectItem(root, "names");
            
            if (names_array && cJSON_IsArray(names_array)) {
                int idx = 0;
                cJSON *item = NULL;
                cJSON_ArrayForEach(item, names_array) {
                    if (idx < 4 && item->valuestring) {
                        relay_names[idx] = String(item->valuestring);
                        preferences.begin("relay_names", false);
                        char key[16];
                        snprintf(key, sizeof(key), "name_%d", idx);
                        preferences.putString(key, relay_names[idx].c_str());
                        preferences.end();
                    }
                    idx++;
                }
                
                cJSON *response = cJSON_CreateObject();
                cJSON_AddNumberToObject(response, "success", 1);
                char *json_str = cJSON_Print(response);
                server.send(200, "application/json", json_str);
                free(json_str);
                cJSON_Delete(response);
                cJSON_Delete(root);
                return;
            }
            cJSON_Delete(root);
        }
    } else if (server.method() == HTTP_GET) {
        cJSON *root = cJSON_CreateObject();
        cJSON *names = cJSON_CreateArray();
        
        for (int i = 0; i < 4; i++) {
            cJSON_AddItemToArray(names, cJSON_CreateString(relay_names[i].c_str()));
        }
        
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
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            wifi_connected = 0;
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            wifi_ip_current = WiFi.localIP().toString();
            break;
        default:
            break;
    }
}

void setup_wifi_sta(String ssid, String password);
void setup_wifi_ap();

void setup_wifi_sta(String ssid, String password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    wifi_ssid_current = ssid;
    
    for (int i = 0; i < 20; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            wifi_connected = 1;
            wifi_ip_current = WiFi.localIP().toString();
            return;
        }
        delay(500);
    }
    setup_wifi_ap();
}

void setup_wifi_ap() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    wifi_ssid_current = WIFI_SSID;
    wifi_ip_current = "192.168.4.1";
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    for (int i = 0; i < 4; i++) {
        pinMode(relay_pins[i], OUTPUT);
        digitalWrite(relay_pins[i], HIGH);
    }
    
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed");
    }
    
    load_translations();
    
    // Load relay names from preferences
    preferences.begin("relay_names", true);
    for (int i = 0; i < 4; i++) {
        char key[16];
        snprintf(key, sizeof(key), "name_%d", i);
        String saved_name = preferences.getString(key, "");
        if (saved_name.length() > 0) {
            relay_names[i] = saved_name;
        }
    }
    preferences.end();
    
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
    
    server.begin();
}

void loop() {
    server.handleClient();
    
    if (WiFi.status() == WL_CONNECTED) {
        wifi_rssi = WiFi.RSSI();
    }
    
    delay(10);
}
