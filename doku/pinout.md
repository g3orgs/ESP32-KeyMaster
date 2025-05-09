# Pinout-Dokumentation (Lolin S3 Mini)

Dieses Dokument beschreibt die verwendete Pin-Belegung im Projekt `TeamsButton6_ESP_SuperMiniS3` basierend auf dem Code in `src/main.cpp`.

## Verwendete Pins

| GPIO Pin | Funktion         | Beschreibung                                      |
|----------|------------------|---------------------------------------------------|
| `GPIO 1` | `BUTTON_A_PIN`   | Eingang für Button A (Taste 'A', INPUT_PULLUP)    |
| `GPIO 3` | `BUTTON_B_PIN`   | Eingang für Button B (Taste 'B', INPUT_PULLUP)    |
| `GPIO 47`| `LED_PIN`        | Ausgang für die Onboard NeoPixel RGB LED          |

## USB Pins

Die Pins für USB (`D-`, `D+`) werden intern vom ESP32-S3 für die USB-OTG-Funktionalität verwendet, wenn `-DARDUINO_USB_MODE=1` gesetzt ist. Sie sind normalerweise nicht direkt für andere Zwecke verfügbar, wenn USB genutzt wird.

## Andere wichtige Pins (aktuell ungenutzt)

*   **Stromversorgung:** 3.3V, GND, 5V (VIN)
*   **Reset:** RST Pin
*   **Boot:** GPIO 0 (oft für Bootloader-Modus verwendet, hier nicht explizit genutzt, aber ggf. durch den Button 0 auf dem Board belegt)
*   **Andere GPIOs:** Viele weitere GPIOs sind verfügbar (siehe Board-Pinout-Diagramm des Herstellers).
*   **I2C:** Standardmäßig oft GPIO 8 (SDA) und 9 (SCL) auf S3 Mini Varianten.
*   **SPI:** Standard-SPI-Pins (SCK, MISO, MOSI, CS) - genaue Pins variieren je nach Board-Definition. 