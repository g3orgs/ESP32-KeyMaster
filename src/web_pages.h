#pragma once

#include <Arduino.h>
#include <pgmspace.h>

// Base pages stored in flash (PROGMEM) to reduce RAM usage.

static const char CONFIG_PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Button Configuration Keymaster </title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>body { font-family: sans-serif; margin: 0; padding: 0; background: #f7f7f7; } .container { max-width: 420px; margin: 20px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px #0001; padding: 20px; } h1, h2 { text-align: center; } label { display: block; margin-top: 10px; font-weight: 600; } input, select { margin-bottom: 10px; width: 100%; padding: 7px; border-radius: 5px; border: 1px solid #ccc; box-sizing: border-box; } .mod-row { display: flex; gap: 8px; margin-bottom: 10px; } .mod-row label { font-weight: 400; margin-top: 0; } .color-row { display: flex; align-items: center; gap: 10px; } .btn-row { display: flex; gap: 10px; justify-content: center; margin-top: 20px; } input[type='submit'] { background: #0078d7; color: #fff; border: none; font-weight: bold; cursor: pointer; transition: background 0.2s; } input[type='submit']:hover { background: #005fa3; } a { color: #0078d7; text-decoration: none; } a:hover { text-decoration: underline; } @media (max-width: 500px) { .container { padding: 10px; } }</style>
<script>function updateColorInputs(id, color) { var r=parseInt(color.substr(1,2),16); var g=parseInt(color.substr(3,2),16); var b=parseInt(color.substr(5,2),16); document.getElementById('r'+id).value=r; document.getElementById('g'+id).value=g; document.getElementById('b'+id).value=b; document.getElementById('hex'+id).value=color.toUpperCase(); } function syncColorPicker(id) { var hex=document.getElementById('hex'+id).value; if(!/^#[0-9A-Fa-f]{6}$/.test(hex)) return; document.getElementById('color'+id).value=hex; var r=parseInt(hex.substr(1,2),16); var g=parseInt(hex.substr(3,2),16); var b=parseInt(hex.substr(5,2),16); document.getElementById('r'+id).value=r; document.getElementById('g'+id).value=g; document.getElementById('b'+id).value=b; }</script>
</head><body><div class='container'><h1>Button Configuration</h1>
<form action='/save' method='POST'>
)rawliteral";

static const char CONFIG_PAGE_FOOT[] PROGMEM = R"rawliteral(
<div class='btn-row'><input type='submit' value='Save Configuration'></div>
</form>
<br><hr><br>
<form action='/exit' method='POST' style='text-align:center;'>
<input type='submit' value='Exit Config Mode & Restart'>
</form>
<br><div style='text-align:center;'><a href='/debug'>Show Stored Values</a> | <a href='/help' target='_blank'>Keycode Help</a> | <a href='/about'>About</a></div>
</div></body></html>
)rawliteral";

static const char SAVE_PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Configuration Saved - Keymaster</title>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>body { font-family: sans-serif; margin: 0; padding: 0; background: #f7f7f7; } .container { max-width: 420px; margin: 20px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px #0001; padding: 20px; } h1 { text-align: center; color: #0078d7; margin-bottom: 10px; } .subtitle { text-align: center; color: #666; margin-bottom: 20px; } .success-icon { text-align: center; font-size: 64px; margin: 20px 0; color: #28a745; } .btn-row { display: flex; gap: 10px; justify-content: center; margin-top: 30px; } a { display: inline-block; padding: 12px 24px; background: #0078d7; color: #fff; text-decoration: none; border-radius: 5px; font-weight: bold; transition: all 0.2s; } a:hover { background: #005fa3; transform: translateY(-1px); box-shadow: 0 2px 4px rgba(0,0,0,0.1); } a.secondary { background: #6c757d; } a.secondary:hover { background: #5a6268; } @media (max-width: 500px) { .container { padding: 15px; margin: 10px; } .btn-row { flex-direction: column; } a { text-align: center; } }</style>
</head><body><div class='container'>
<h1>Configuration Saved!</h1>
<p class='subtitle'>Your Keymaster settings have been updated successfully</p>
<div class='success-icon'>✓</div>
<div class='btn-row'>
<a href='/'>Back to Configuration</a>
)rawliteral";

static const char SAVE_PAGE_FOOT[] PROGMEM = R"rawliteral(
</div></div></body></html>
)rawliteral";

static const char EXIT_PAGE[] PROGMEM = R"rawliteral(
<html><body><h1>Exiting Config Mode</h1><p>Device will restart in normal mode.</p></body></html>
)rawliteral";

static const char DEBUG_PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Debug Config</title>
<style>body { font-family: sans-serif; } th, td { padding: 5px; text-align: left; border-bottom: 1px solid #ddd; }</style>
</head><body><h1>Current Stored Configuration</h1>
<table>
<tr><th>Parameter</th><th>Button A</th><th>Button B</th></tr>
)rawliteral";

static const char DEBUG_PAGE_FOOT[] PROGMEM = R"rawliteral(
</table>
<br><a href='/'>Back to Config</a>
</body></html>
)rawliteral";

static const char HELP_PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>HID Keycode Help</title>
<style>body { font-family: sans-serif; } table { border-collapse: collapse; } th, td { border: 1px solid #ccc; padding: 4px; } th { background: #eee; }</style>
</head><body><h1>HID Keycode Reference</h1>
<p>Use the following keycodes for configuration. For letters and numbers, just use the decimal value. For special keys, see the table below.</p>
)rawliteral";

static const char HELP_PAGE_FOOT[] PROGMEM = R"rawliteral(
<br><a href='/'>Back to Configuration</a>
</body></html>
)rawliteral";

static const char ABOUT_PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>About - Keymaster</title>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>body { font-family: sans-serif; margin: 0; padding: 0; background: #f7f7f7; } .container { max-width: 420px; margin: 20px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px #0001; padding: 20px; } h1 { text-align: center; color: #0078d7; margin-bottom: 10px; } .subtitle { text-align: center; color: #666; margin-bottom: 20px; } .version { text-align: center; font-size: 1.2em; margin: 20px 0; padding: 10px; background: #f8f9fa; border-radius: 5px; } .credits { margin-top: 20px; padding-top: 20px; border-top: 1px solid #eee; } .credits h2 { font-size: 1.2em; color: #666; } .credits p { color: #666; line-height: 1.6; } .btn-row { display: flex; gap: 10px; justify-content: center; margin-top: 30px; } a { display: inline-block; padding: 12px 24px; background: #0078d7; color: #fff; text-decoration: none; border-radius: 5px; font-weight: bold; transition: all 0.2s; } a:hover { background: #005fa3; transform: translateY(-1px); box-shadow: 0 2px 4px rgba(0,0,0,0.1); } @media (max-width: 500px) { .container { padding: 15px; margin: 10px; } }</style>
</head><body><div class='container'>
<h1>About Keymaster</h1>
<div class='version'>Version 
)rawliteral";

static const char ABOUT_PAGE_MID[] PROGMEM = R"rawliteral(
</div>
<div class='credits'>
<h2>Credits</h2>
<p>Keymaster is an open-source project that allows you to create custom keyboard shortcuts using ESP32.</p>
<p>More credits and information coming soon...</p>
</div>
<div class='btn-row'>
<a href='/'>Back to Configuration</a>
</div></div></body></html>
)rawliteral";
