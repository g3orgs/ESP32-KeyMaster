# Feature Requests (Step-by-Step, Safe)

Each item is independent. After finishing one item, run the listed tests/validation before moving to the next. The goal is to preserve original functionality and avoid crashes.

1. Non-blocking debounce and button state machine (remove `delay()`-based debounce in `loop()`).
Tests:
- Build: `pio run -e esp32_s3_super_mini`
- Manual: tap Button A and B rapidly; each press should register once.
- Manual: hold Button B for 5s; AP mode should enter reliably.

2. Debug mode gated by a compile-time flag (default OFF).
Tests:
- Build: `pio run -e esp32_s3_super_mini`
- Manual: with flag OFF, HID and AP mode work as before.
- Manual: with flag ON, LED shows button states and HID/AP are disabled.

3. Web UI input validation (keycodes, modifier mask, RGB 0-255 clamp).
Tests:
- Build: `pio run -e esp32_s3_super_mini`
- Manual: enter out-of-range values; saved values should be clamped.
- Manual: saved values must persist after reboot.

4. HTML in PROGMEM / header file (reduce heap fragmentation, improve maintainability).
Tests:
- Build: `pio run -e esp32_s3_super_mini`
- Manual: AP page loads correctly, forms submit, and styles remain intact.

5. Clean exit from config mode without `ESP.restart()`.
Tests:
- Build: `pio run -e esp32_s3_super_mini`
- Manual: enter AP mode, save config, exit; device returns to normal mode.
- Manual: HID sends keycodes after exit.

6. Optional AP password support (documented).
Tests:
- Build: `pio run -e esp32_s3_super_mini`
- Manual: AP requires password when enabled; open when disabled.
- Manual: config page remains accessible and works.
