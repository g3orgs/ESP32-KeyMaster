# Pinout-Dokumentation (ESP32-S3 Super Mini)

Dieses Dokument beschreibt die verwendete Pin-Belegung im Projekt basierend auf dem Code in `src/main.cpp` für das ESP32-S3 Super Mini.

## Verwendete Pins

| GPIO Pin | Funktion         | Beschreibung                                      |
|----------|------------------|---------------------------------------------------|
| `GPIO 1` | `BUTTON_A_PIN`   | Eingang für Button A (INPUT_PULLUP)               |
| `GPIO 4` | `BUTTON_B_PIN`   | Eingang für Button B (INPUT_PULLUP)               |
| `GPIO 48`| `LED_PIN`        | Ausgang für die Onboard WS2812 RGB LED            |

Hinweis: GPIO 48 teilt sich die Leitung mit der roten Power-LED. Bei WS2812-Aktivität kann die rote LED mitblinken.

## USB Pins

Die Pins für USB (`D-`, `D+`) werden intern vom ESP32-S3 für die USB-OTG-Funktionalität verwendet, wenn `-DARDUINO_USB_MODE=1` gesetzt ist. Sie sind normalerweise nicht direkt für andere Zwecke verfügbar, wenn USB genutzt wird.

## Andere wichtige Pins (aktuell ungenutzt)

*   **Stromversorgung:** 3.3V, GND, 5V (VIN)
*   **Reset:** RST Pin
*   **Boot:** GPIO 0 (oft für Bootloader-Modus verwendet, hier nicht explizit genutzt, aber ggf. durch den Button 0 auf dem Board belegt)
*   **Andere GPIOs:** Viele weitere GPIOs sind verfügbar (siehe Board-Pinout-Diagramm des Herstellers).
*   **I2C:** Standardmäßig oft GPIO 8 (SDA) und 9 (SCL) auf S3 Mini Varianten.
*   **SPI:** Standard-SPI-Pins (SCK, MISO, MOSI, CS) - genaue Pins variieren je nach Board-Definition. 
