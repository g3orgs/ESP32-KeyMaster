#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <USB.h>             // Core USB-Bibliothek
#include <USBHIDKeyboard.h>  // Core HID Keyboard
#include <WiFi.h>            // Für AP-Modus
#include <WebServer.h>       // Für Konfigurationsseite
#include <Preferences.h>     // Für persistente Speicherung

#define LED_PIN 48
#define NUMPIXELS 1
#define BUTTON_A_PIN 1 // Button für Taste/Kombi A
#define BUTTON_B_PIN 4 // Button für Taste/Kombi B (und Konfig-Aktivierung)
#define FW_VERSION "1.2.0"
#ifndef DEBUG_BUTTON_MODE
#define DEBUG_BUTTON_MODE 0 // 1 = Button-States per LED anzeigen, HID/Config deaktiviert
#endif

// --- Globale Konfiguration ---
struct ButtonConfig {
  uint8_t key;      // HID Keycode (z.B. KEY_A, KEY_M, KEY_F1)
  uint8_t modifier; // Bitmaske für Modifier (KEY_LEFT_CTRL, KEY_LEFT_SHIFT, etc.)
  uint8_t r, g, b;  // LED Farbe
};

// Initialisiere die globalen Konfigurationsvariablen direkt mit Defaults
ButtonConfig buttonA_config = {0x04, 0, 255, 255, 255}; // Default A
ButtonConfig buttonB_config = {0x05, 0, 0, 0, 255};     // Default B

// Die const-Varianten behalten wir als Referenz oder für loadConfiguration
const ButtonConfig default_A_config = {0x04, 0, 255, 255, 255}; // HID Code 'a'/'A', kein expliziter Modifier, Weiß
const ButtonConfig default_B_config = {0x05, 0, 0, 0, 255};     // HID Code 'b'/'B', kein expliziter Modifier, Blau

Preferences preferences; // Objekt für NVS

// --- Zustandsvariablen ---
bool configModeActive = false; // Ist der Konfigurationsmodus aktiv?
unsigned long buttonB_pressStartTime = 0; // Zeitpunkt, wann B gedrückt wurde
const unsigned long longPressDuration = 5000; // 5 Sekunden für langen Druck
const unsigned long debounceDuration = 30; // Entprellzeit in ms
unsigned long ledPulseUntil = 0;
uint32_t ledPulseColor = 0;
// ------------------------

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
USBHIDKeyboard Keyboard; // Keyboard-Objekt aus der Core-Bibliothek

// --- Debounce-Helper ---
struct DebouncedButton {
  uint8_t pin;
  int stableState;
  int lastReading;
  unsigned long lastChange;
  bool fell;
  bool rose;
};

DebouncedButton btnA = {BUTTON_A_PIN, HIGH, HIGH, 0, false, false};
DebouncedButton btnB = {BUTTON_B_PIN, HIGH, HIGH, 0, false, false};

void updateButton(DebouncedButton &btn, unsigned long now) {
  btn.fell = false;
  btn.rose = false;
  int reading = digitalRead(btn.pin);
  if (reading != btn.lastReading) {
    btn.lastChange = now;
    btn.lastReading = reading;
  }
  if ((now - btn.lastChange) >= debounceDuration && reading != btn.stableState) {
    btn.stableState = reading;
    if (btn.stableState == LOW) {
      btn.fell = true;
    } else {
      btn.rose = true;
    }
  }
}

// USB Event Callback (benötigt, aber Inhalt für uns nicht relevant)
void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
  // Leer lassen oder minimale Logik bei Bedarf
}

// --- Forward Declarations ---
void loadConfiguration();
void saveConfiguration();
void startAPMode();
void stopAPMode();
void enterConfigMode();
void exitConfigMode();
void setupWebServerRoutes();
String getHtmlContent();
void handleRoot();
void handleSave();
void handleExit();
void handleDebug();
void handleHelp();
void handleAbout(); // Neue Forward Declaration
// --------------------------

// --- WiFi / Webserver Objekte ---
const char* ap_ssid = "ESP32-KeyMaster";
// const char* ap_password = "password"; // Optional: Passwort setzen
WebServer server(80);
// -----------------------------

// --- Hilfsfunktion: Sende HID-Report mit Modifiern und Keycode ---
void sendHIDKey(uint8_t modifier, uint8_t key) {
  KeyReport report = {0};
  report.modifiers = modifier;
  report.keys[0] = key;
  Keyboard.sendReport(&report);
  delay(10);
  KeyReport release = {0};
  Keyboard.sendReport(&release);
}

// Hilfsfunktion: Zahl zu zweistelligem Hex-String
String hex2(uint8_t v) {
  String s = String(v, HEX);
  if (s.length() < 2) s = "0" + s;
  return s;
}

// Clamp integer to 0..255
uint8_t clampByte(int v) {
  if (v < 0) return 0;
  if (v > 255) return 255;
  return static_cast<uint8_t>(v);
}

// --- Funktionen zum Laden/Speichern der Konfiguration ---
void loadConfiguration() {
  preferences.begin("keyconfig", true); // Namespace "keyconfig", read-only erstmal (wird false zum Schreiben)

  buttonA_config.key      = preferences.getUChar("keyA", default_A_config.key);
  buttonA_config.modifier = preferences.getUChar("modA", default_A_config.modifier);
  buttonA_config.r        = preferences.getUChar("rA", default_A_config.r);
  buttonA_config.g        = preferences.getUChar("gA", default_A_config.g);
  buttonA_config.b        = preferences.getUChar("bA", default_A_config.b);

  buttonB_config.key      = preferences.getUChar("keyB", default_B_config.key);
  buttonB_config.modifier = preferences.getUChar("modB", default_B_config.modifier);
  buttonB_config.r        = preferences.getUChar("rB", default_B_config.r);
  buttonB_config.g        = preferences.getUChar("gB", default_B_config.g);
  buttonB_config.b        = preferences.getUChar("bB", default_B_config.b);

  preferences.end();
}

void saveConfiguration() {
  preferences.begin("keyconfig", false); // Namespace "keyconfig", read/write

  preferences.putUChar("keyA", buttonA_config.key);
  preferences.putUChar("modA", buttonA_config.modifier);
  preferences.putUChar("rA", buttonA_config.r);
  preferences.putUChar("gA", buttonA_config.g);
  preferences.putUChar("bA", buttonA_config.b);

  preferences.putUChar("keyB", buttonB_config.key);
  preferences.putUChar("modB", buttonB_config.modifier);
  preferences.putUChar("rB", buttonB_config.r);
  preferences.putUChar("gB", buttonB_config.g);
  preferences.putUChar("bB", buttonB_config.b);

  preferences.end();
}
// ------------------------------------------------------

// --- WiFi / AP / Config Mode Funktionen ---
void startAPMode() {
  // Optional: Bestehende WiFi-Verbindung trennen, falls vorhanden
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
  // WiFi.softAP(ap_ssid, ap_password); // Mit Passwort
  WiFi.softAP(ap_ssid); // Ohne Passwort
}

void stopAPMode() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF); // Oder WIFI_STA, falls später wieder Client-Modus benötigt wird
}

void enterConfigMode() {
  if (configModeActive) return; // Nicht erneut starten, wenn schon aktiv

  configModeActive = true;
  buttonB_pressStartTime = 0; // Reset timer

  // USB Keyboard deaktivieren? (Kann Konflikte vermeiden)
  // Keyboard.end(); 

  // Signalisiere Konfig-Modus (Gelb)
  pixels.setPixelColor(0, pixels.Color(255, 150, 0));
  pixels.show();

  startAPMode();

  // Webserver starten
  setupWebServerRoutes(); // Routen definieren
  server.begin();         // Server starten
}

void exitConfigMode() {
  if (!configModeActive) return;

  // Webserver stoppen
  server.stop();

  stopAPMode();

  // Zurück zum normalen Modus
  pixels.clear();
  pixels.show();
  configModeActive = false;

  // USB Keyboard wieder aktivieren? (Wenn oben beendet)
  // Keyboard.begin();

  delay(100); // Kurze Pause geben
  ESP.restart(); // Sicherstellen, dass alles neu initialisiert wird
}
// ----------------------------------------

// Funktion zum Erstellen des HTML-Inhalts für die Konfigurationsseite
String getHtmlContent() {
  String html = "<!DOCTYPE html><html><head><title>Button Configuration Keymaster </title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body { font-family: sans-serif; margin: 0; padding: 0; background: #f7f7f7; } .container { max-width: 420px; margin: 20px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px #0001; padding: 20px; } h1, h2 { text-align: center; } label { display: block; margin-top: 10px; font-weight: 600; } input, select { margin-bottom: 10px; width: 100%; padding: 7px; border-radius: 5px; border: 1px solid #ccc; box-sizing: border-box; } .mod-row { display: flex; gap: 8px; margin-bottom: 10px; } .mod-row label { font-weight: 400; margin-top: 0; } .color-row { display: flex; align-items: center; gap: 10px; } .btn-row { display: flex; gap: 10px; justify-content: center; margin-top: 20px; } input[type='submit'] { background: #0078d7; color: #fff; border: none; font-weight: bold; cursor: pointer; transition: background 0.2s; } input[type='submit']:hover { background: #005fa3; } a { color: #0078d7; text-decoration: none; } a:hover { text-decoration: underline; } @media (max-width: 500px) { .container { padding: 10px; } }</style>";
  html += "<script>function updateColorInputs(id, color) { var r=parseInt(color.substr(1,2),16); var g=parseInt(color.substr(3,2),16); var b=parseInt(color.substr(5,2),16); document.getElementById('r'+id).value=r; document.getElementById('g'+id).value=g; document.getElementById('b'+id).value=b; document.getElementById('hex'+id).value=color.toUpperCase(); } function syncColorPicker(id) { var hex=document.getElementById('hex'+id).value; if(!/^#[0-9A-Fa-f]{6}$/.test(hex)) return; document.getElementById('color'+id).value=hex; var r=parseInt(hex.substr(1,2),16); var g=parseInt(hex.substr(3,2),16); var b=parseInt(hex.substr(5,2),16); document.getElementById('r'+id).value=r; document.getElementById('g'+id).value=g; document.getElementById('b'+id).value=b; }</script>";
  html += "</head><body><div class='container'><h1>Button Configuration</h1>";
  html += "<form action='/save' method='POST'>";

  // --- Button A Configuration ---
  html += "<h2>Button A</h2>";
  html += "<label for='keyA'>Key Code (HID):</label>";
  html += "<input type='number' id='keyA' name='keyA' value='" + String(buttonA_config.key) + "' required> <a href='/help' target='_blank'>[Keycode Help]</a>";
  html += "<label>Modifier:</label>";
  html += "<div class='mod-row'>";
  html += "<label><input type='checkbox' id='modA_ctrl' name='modA_ctrl' value='1'" + String((buttonA_config.modifier & 0x01) ? " checked" : "") + ">Ctrl</label>";
  html += "<label><input type='checkbox' id='modA_shift' name='modA_shift' value='2'" + String((buttonA_config.modifier & 0x02) ? " checked" : "") + ">Shift</label>";
  html += "<label><input type='checkbox' id='modA_alt' name='modA_alt' value='4'" + String((buttonA_config.modifier & 0x04) ? " checked" : "") + ">Alt</label>";
  html += "<label><input type='checkbox' id='modA_gui' name='modA_gui' value='8'" + String((buttonA_config.modifier & 0x08) ? " checked" : "") + ">GUI</label>";
  html += "</div>";
  html += "<label for='colorA'>LED Color:</label>";
  String hexA = "#" + hex2(buttonA_config.r) + hex2(buttonA_config.g) + hex2(buttonA_config.b);
  html += "<div class='color-row'><input type='color' id='colorA' value='"+hexA+"' onchange=\"updateColorInputs('A',this.value)\"><input type='text' id='hexA' value='"+hexA+"' maxlength='7' style='width:90px;text-transform:uppercase;' oninput=\"syncColorPicker('A')\"></div>";
  html += "<input type='hidden' id='rA' name='rA' value='"+String(buttonA_config.r)+"'>";
  html += "<input type='hidden' id='gA' name='gA' value='"+String(buttonA_config.g)+"'>";
  html += "<input type='hidden' id='bA' name='bA' value='"+String(buttonA_config.b)+"'>";

  // --- Button B Configuration ---
  html += "<h2>Button B</h2>";
  html += "<label for='keyB'>Key Code (HID):</label>";
  html += "<input type='number' id='keyB' name='keyB' value='" + String(buttonB_config.key) + "' required> <a href='/help' target='_blank'>[Keycode Help]</a>";
  html += "<label>Modifier:</label>";
  html += "<div class='mod-row'>";
  html += "<label><input type='checkbox' id='modB_ctrl' name='modB_ctrl' value='1'" + String((buttonB_config.modifier & 0x01) ? " checked" : "") + ">Ctrl</label>";
  html += "<label><input type='checkbox' id='modB_shift' name='modB_shift' value='2'" + String((buttonB_config.modifier & 0x02) ? " checked" : "") + ">Shift</label>";
  html += "<label><input type='checkbox' id='modB_alt' name='modB_alt' value='4'" + String((buttonB_config.modifier & 0x04) ? " checked" : "") + ">Alt</label>";
  html += "<label><input type='checkbox' id='modB_gui' name='modB_gui' value='8'" + String((buttonB_config.modifier & 0x08) ? " checked" : "") + ">GUI</label>";
  html += "</div>";
  html += "<label for='colorB'>LED Color:</label>";
  String hexB = "#" + hex2(buttonB_config.r) + hex2(buttonB_config.g) + hex2(buttonB_config.b);
  html += "<div class='color-row'><input type='color' id='colorB' value='"+hexB+"' onchange=\"updateColorInputs('B',this.value)\"><input type='text' id='hexB' value='"+hexB+"' maxlength='7' style='width:90px;text-transform:uppercase;' oninput=\"syncColorPicker('B')\"></div>";
  html += "<input type='hidden' id='rB' name='rB' value='"+String(buttonB_config.r)+"'>";
  html += "<input type='hidden' id='gB' name='gB' value='"+String(buttonB_config.g)+"'>";
  html += "<input type='hidden' id='bB' name='bB' value='"+String(buttonB_config.b)+"'>";

  html += "<div class='btn-row'><input type='submit' value='Save Configuration'></div>";
  html += "</form>";

  // --- Option to exit config mode ---
  html += "<br><hr><br>";
  html += "<form action='/exit' method='POST' style='text-align:center;'>";
  html += "<input type='submit' value='Exit Config Mode & Restart'>";
  html += "</form>";
  // --- Link to Debug and Help page ---
  html += "<br><div style='text-align:center;'><a href='/debug'>Show Stored Values</a> | <a href='/help' target='_blank'>Keycode Help</a> | <a href='/about'>About</a></div>";
  html += "</div></body></html>";
  return html;
}

// Funktion zum Behandeln der Hauptseite ("/")
void handleRoot() {
  server.send(200, "text/html", getHtmlContent());
}

// Funktion zum Behandeln des Speicherns der Konfiguration ("/save")
void handleSave() {
  // Modifier-Bitmasken aus Checkboxen berechnen
  uint8_t modA = 0;
  uint8_t modB = 0;
  if (server.hasArg("modA_ctrl")) modA |= 0x01;
  if (server.hasArg("modA_shift")) modA |= 0x02;
  if (server.hasArg("modA_alt")) modA |= 0x04;
  if (server.hasArg("modA_gui")) modA |= 0x08;
  if (server.hasArg("modB_ctrl")) modB |= 0x01;
  if (server.hasArg("modB_shift")) modB |= 0x02;
  if (server.hasArg("modB_alt")) modB |= 0x04;
  if (server.hasArg("modB_gui")) modB |= 0x08;

  if (server.hasArg("keyA") && server.hasArg("rA") && server.hasArg("gA") && server.hasArg("bA") &&
      server.hasArg("keyB") && server.hasArg("rB") && server.hasArg("gB") && server.hasArg("bB")) {

    // Button A Daten extrahieren
    buttonA_config.key = clampByte(server.arg("keyA").toInt());
    buttonA_config.modifier = modA & 0x0F;
    buttonA_config.r = clampByte(server.arg("rA").toInt());
    buttonA_config.g = clampByte(server.arg("gA").toInt());
    buttonA_config.b = clampByte(server.arg("bA").toInt());

    // Button B Daten extrahieren
    buttonB_config.key = clampByte(server.arg("keyB").toInt());
    buttonB_config.modifier = modB & 0x0F;
    buttonB_config.r = clampByte(server.arg("rB").toInt());
    buttonB_config.g = clampByte(server.arg("gB").toInt());
    buttonB_config.b = clampByte(server.arg("bB").toInt());

    saveConfiguration(); // Speichere die neue Konfiguration

    // Neue, schönere Bestätigungsseite
    String html = "<!DOCTYPE html><html><head><title>Configuration Saved - Keymaster</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body { font-family: sans-serif; margin: 0; padding: 0; background: #f7f7f7; } .container { max-width: 420px; margin: 20px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px #0001; padding: 20px; } h1 { text-align: center; color: #0078d7; margin-bottom: 10px; } .subtitle { text-align: center; color: #666; margin-bottom: 20px; } .success-icon { text-align: center; font-size: 64px; margin: 20px 0; color: #28a745; } .btn-row { display: flex; gap: 10px; justify-content: center; margin-top: 30px; } a { display: inline-block; padding: 12px 24px; background: #0078d7; color: #fff; text-decoration: none; border-radius: 5px; font-weight: bold; transition: all 0.2s; } a:hover { background: #005fa3; transform: translateY(-1px); box-shadow: 0 2px 4px rgba(0,0,0,0.1); } a.secondary { background: #6c757d; } a.secondary:hover { background: #5a6268; } @media (max-width: 500px) { .container { padding: 15px; margin: 10px; } .btn-row { flex-direction: column; } a { text-align: center; } }</style>";
    html += "</head><body><div class='container'>";
    html += "<h1>Configuration Saved!</h1>";
    html += "<p class='subtitle'>Your Keymaster settings have been updated successfully</p>";
    html += "<div class='success-icon'>✓</div>";
    html += "<div class='btn-row'>";
    html += "<a href='/'>Back to Configuration</a>";
    html += "</div></div></body></html>";

    server.send(200, "text/html", html);

  } else {
    server.send(400, "text/plain", "Bad Request: Missing parameters");
  }
}

// Funktion zum Behandeln des Verlassens des Konfig-Modus ("/exit")
void handleExit() {
  server.send(200, "text/html", "<html><body><h1>Exiting Config Mode</h1><p>Device will restart in normal mode.</p></body></html>");
  delay(100);
  exitConfigMode();
}

// --- NEUE Funktion zum Behandeln der Debug-Seite ("/debug") ---
void handleDebug() {
  // Lade die aktuell *gespeicherten* Werte, um sicher zu sein
  loadConfiguration(); 

  String html = "<!DOCTYPE html><html><head><title>Debug Config</title>";
  html += "<style>body { font-family: sans-serif; } th, td { padding: 5px; text-align: left; border-bottom: 1px solid #ddd; }</style>";
  html += "</head><body><h1>Current Stored Configuration</h1>";
  html += "<table>";
  html += "<tr><th>Parameter</th><th>Button A</th><th>Button B</th></tr>";
  html += "<tr><td>Key Code (HID)</td><td>" + String(buttonA_config.key) + "</td><td>" + String(buttonB_config.key) + "</td></tr>";
  html += "<tr><td>Modifier (Bitmask)</td><td>" + String(buttonA_config.modifier) + "</td><td>" + String(buttonB_config.modifier) + "</td></tr>";
  html += "<tr><td>LED Red</td><td>" + String(buttonA_config.r) + "</td><td>" + String(buttonB_config.r) + "</td></tr>";
  html += "<tr><td>LED Green</td><td>" + String(buttonA_config.g) + "</td><td>" + String(buttonB_config.g) + "</td></tr>";
  html += "<tr><td>LED Blue</td><td>" + String(buttonA_config.b) + "</td><td>" + String(buttonB_config.b) + "</td></tr>";
  html += "</table>";
  html += "<br><a href='/'>Back to Config</a>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
// ---------------------------------------------------------

// --- Neue Funktion: Hilfeseite mit Keycode-Tabelle ---
void handleHelp() {
  String html = "<!DOCTYPE html><html><head><title>HID Keycode Help</title>";
  html += "<style>body { font-family: sans-serif; } table { border-collapse: collapse; } th, td { border: 1px solid #ccc; padding: 4px; } th { background: #eee; }</style>";
  html += "</head><body><h1>HID Keycode Reference</h1>";
  html += "<p>Use the following keycodes for configuration. For letters and numbers, just use the decimal value. For special keys, see the table below.</p>";
  html += "<h2>Letters (A-Z)</h2>";
  html += "<table><tr><th>Letter</th><th>Keycode</th></tr>";
  for (int i = 0; i < 26; i++) {
    html += "<tr><td>"; html += char('A'+i); html += "</td><td>"; html += String(4+i); html += "</td></tr>";
  }
  html += "</table>";
  html += "<h2>Numbers (1-0)</h2>";
  html += "<table><tr><th>Number</th><th>Keycode</th></tr>";
  for (int i = 0; i < 9; i++) {
    html += "<tr><td>"; html += String(i+1); html += "</td><td>"; html += String(30+i); html += "</td></tr>";
  }
  html += "<tr><td>0</td><td>39</td></tr></table>";
  html += "<h2>Function Keys</h2>";
  html += "<table><tr><th>Key</th><th>Keycode</th></tr>";
  for (int i = 0; i < 12; i++) {
    html += "<tr><td>F"; html += String(i+1); html += "</td><td>"; html += String(58+i); html += "</td></tr>";
  }
  html += "</table>";
  html += "<h2>Special Keys</h2>";
  html += "<table><tr><th>Key</th><th>Keycode</th></tr>";
  html += "<tr><td>Enter</td><td>40</td></tr>";
  html += "<tr><td>Escape</td><td>41</td></tr>";
  html += "<tr><td>Backspace</td><td>42</td></tr>";
  html += "<tr><td>Tab</td><td>43</td></tr>";
  html += "<tr><td>Space</td><td>44</td></tr>";
  html += "<tr><td>Minus (-)</td><td>45</td></tr>";
  html += "<tr><td>Equal (=)</td><td>46</td></tr>";
  html += "<tr><td>[</td><td>47</td></tr>";
  html += "<tr><td>]</td><td>48</td></tr>";
  html += "<tr><td>Backslash (\\)</td><td>49</td></tr>";
  html += "<tr><td>Semicolon (;)</td><td>51</td></tr>";
  html += "<tr><td>Quote (')</td><td>52</td></tr>";
  html += "<tr><td>Grave (`)</td><td>53</td></tr>";
  html += "<tr><td>Comma (,)</td><td>54</td></tr>";
  html += "<tr><td>Period (.)</td><td>55</td></tr>";
  html += "<tr><td>Slash (/)</td><td>56</td></tr>";
  html += "</table>";
  html += "<h2>Modifier Keys (Checkboxes)</h2>";
  html += "<table><tr><th>Modifier</th><th>Description</th></tr>";
  html += "<tr><td>Ctrl</td><td>Control (Strg, Command on Mac)</td></tr>";
  html += "<tr><td>Shift</td><td>Shift</td></tr>";
  html += "<tr><td>Alt</td><td>Alt (Option on Mac)</td></tr>";
  html += "<tr><td>GUI</td><td>Windows key (Win), Command (Mac), Super (Linux)</td></tr>";
  html += "</table>";
  html += "<br><a href='/'>Back to Configuration</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Neue About-Seite Handler
void handleAbout() {
  String html = "<!DOCTYPE html><html><head><title>About - Keymaster</title>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body { font-family: sans-serif; margin: 0; padding: 0; background: #f7f7f7; } .container { max-width: 420px; margin: 20px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px #0001; padding: 20px; } h1 { text-align: center; color: #0078d7; margin-bottom: 10px; } .subtitle { text-align: center; color: #666; margin-bottom: 20px; } .version { text-align: center; font-size: 1.2em; margin: 20px 0; padding: 10px; background: #f8f9fa; border-radius: 5px; } .credits { margin-top: 20px; padding-top: 20px; border-top: 1px solid #eee; } .credits h2 { font-size: 1.2em; color: #666; } .credits p { color: #666; line-height: 1.6; } .btn-row { display: flex; gap: 10px; justify-content: center; margin-top: 30px; } a { display: inline-block; padding: 12px 24px; background: #0078d7; color: #fff; text-decoration: none; border-radius: 5px; font-weight: bold; transition: all 0.2s; } a:hover { background: #005fa3; transform: translateY(-1px); box-shadow: 0 2px 4px rgba(0,0,0,0.1); } @media (max-width: 500px) { .container { padding: 15px; margin: 10px; } }</style>";
  html += "</head><body><div class='container'>";
  html += "<h1>About Keymaster</h1>";
  html += "<div class='version'>Version ";
  html += FW_VERSION;
  html += "</div>";
  html += "<div class='credits'>";
  html += "<h2>Credits</h2>";
  html += "<p>Keymaster is an open-source project that allows you to create custom keyboard shortcuts using ESP32.</p>";
  html += "<p>More credits and information coming soon...</p>";
  html += "</div>";
  html += "<div class='btn-row'>";
  html += "<a href='/'>Back to Configuration</a>";
  html += "</div></div></body></html>";
  
  server.send(200, "text/html", html);
}

// Funktion zum Einrichten der Webserver-Routen
void setupWebServerRoutes() {
  server.on("/", HTTP_GET, handleRoot);      // Hauptseite anzeigen
  server.on("/save", HTTP_POST, handleSave); // Konfiguration speichern
  server.on("/exit", HTTP_POST, handleExit); // Konfigurationsmodus verlassen
  server.on("/debug", HTTP_GET, handleDebug); // Route für Debug-Seite
  server.on("/help", HTTP_GET, handleHelp);   // Hilfe-Seite
  server.on("/about", HTTP_GET, handleAbout); // Neue About-Seite
}

void setup() {
  // NeoPixel initialisieren
  pixels.begin();
  pixels.setBrightness(20);
  pixels.clear();
  pixels.show();
  
  // Konfiguration laden
  // Hinweis: Da aktuell Keyboard.print() verwendet wird, werden nur die
  // konfigurierten LED-Farben geladen. Tasten/Modifier aus der Config
  // werden NICHT verwendet (sendet immer 'A'/'B').
  loadConfiguration(); 

  // Button-Pins als Eingang mit Pull-up konfigurieren
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);

  // USB initialisieren
  USB.onEvent(usbEventCallback); // Event-Handler registrieren
  Keyboard.begin();             // Tastatur-HID starten
  USB.begin();                  // USB-Stack starten

  // Kurze Anzeige, dass Setup fertig ist
  pixels.setPixelColor(0, pixels.Color(0, 20, 0)); // Grün
  pixels.show();
  delay(500);
  pixels.clear();
  pixels.show();
}

void loop() {
  unsigned long now = millis();
  // Lese die aktuellen Zustände der Buttons (entprellt)
  updateButton(btnA, now);
  updateButton(btnB, now);

#if DEBUG_BUTTON_MODE
  // Debug: LED zeigt Button-Zustände, HID/AP sind deaktiviert
  if (btnA.stableState == LOW && btnB.stableState == LOW) {
    pixels.setPixelColor(0, pixels.Color(255, 255, 0)); // Beide gedrückt = Gelb
  } else if (btnA.stableState == LOW) {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // A gedrückt = Grün
  } else if (btnB.stableState == LOW) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // B gedrückt = Rot
  } else {
    pixels.clear(); // Keiner gedrückt
  }
  pixels.show();
  return;
#endif

  // --- Konfigurationsmodus Handling ---
  if (configModeActive) {
    // Im Konfigurationsmodus: Webserver-Anfragen bearbeiten
    server.handleClient();

    // LED dauerhaft gelb signalisiert Konfig-Modus
    pixels.setPixelColor(0, pixels.Color(255, 150, 0));
    pixels.show();

    // Im Konfigurationsmodus werden normale Button-Aktionen übersprungen
    return; // Schleife hier beenden für den Konfig-Modus
  }

  // --- Normaler Betriebsmodus ---

  // --- Logik für Button A (nur kurzer Druck) ---
  if (btnA.fell) {
    // Flanke erkannt (gerade gedrückt)
    sendHIDKey(buttonA_config.modifier, buttonA_config.key);
    // Visuelles Feedback (konfigurierte Farbe A wird verwendet!)
    ledPulseColor = pixels.Color(buttonA_config.r, buttonA_config.g, buttonA_config.b);
    ledPulseUntil = now + 100;
  }

  // --- Logik für Button B (kurzer Druck vs. langer Druck) ---
  // 1. Langen Druck erkennen
  if (btnB.fell) {
    // Flanke erkannt (gerade gedrückt) -> Starte Timer für langen Druck
    buttonB_pressStartTime = now;
  }
  if (btnB.stableState == LOW && buttonB_pressStartTime != 0 &&
      (now - buttonB_pressStartTime > longPressDuration)) {
    // Langer Druck erkannt! -> Konfigurationsmodus starten
    enterConfigMode();
    buttonB_pressStartTime = 0;
    return;
  }
  if (btnB.rose) {
    // Flanke erkannt (gerade losgelassen)
    if (buttonB_pressStartTime != 0) {
      // Es war ein kurzer Druck (langer Druck hätte Timer auf 0 gesetzt)
      sendHIDKey(buttonB_config.modifier, buttonB_config.key);
      // Visuelles Feedback (konfigurierte Farbe B wird verwendet!)
      ledPulseColor = pixels.Color(buttonB_config.r, buttonB_config.g, buttonB_config.b);
      ledPulseUntil = now + 100;
    }
    buttonB_pressStartTime = 0;
  }

  // LED-Puls (nicht-blockierend)
  if (ledPulseUntil != 0) {
    if (now <= ledPulseUntil) {
      pixels.setPixelColor(0, ledPulseColor);
      pixels.show();
    } else {
      pixels.clear();
      pixels.show();
      ledPulseUntil = 0;
    }
  }
}
