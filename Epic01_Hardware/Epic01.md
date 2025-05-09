# Epic 1: Grundlegende Funktionalität

## User Stories

### US1.1: ESP32-S3 als USB-HID-Tastatur
- Als Benutzer möchte ich, dass der ESP32-S3 als USB-HID-Tastatur erkannt wird
- Akzeptanzkriterien:
  - ESP32-S3 wird vom System als USB-HID-Tastatur erkannt
  - Korrekte USB-Deskriptoren sind implementiert
  - USB-Verbindung ist stabil

### US1.2: Hardware-Buttons
- Als Benutzer möchte ich zwei physische Buttons (A und B) nutzen können
- Akzeptanzkriterien:
  - Beide Buttons sind zuverlässig ansprechbar
  - Entprellung ist implementiert
  - Klare physische Unterscheidung der Buttons

### US1.3: LED-Feedback
- Als Benutzer möchte ich visuelles Feedback durch LEDs erhalten
- Akzeptanzkriterien:
  - LED zeigt Status der Button-Aktionen
  - Verschiedene Farben für verschiedene Zustände
  - Sichtbares Feedback bei Konfigurationsänderungen

### US1.4: Konfigurationsmodus
- Als Benutzer möchte ich durch langes Drücken von Button B in den Konfigurationsmodus gelangen
- Akzeptanzkriterien:
  - Zuverlässige Erkennung des langen Drückens
  - Klare visuelle Rückmeldung beim Moduswechsel
  - Einfaches Verlassen des Konfigurationsmodus

### US1.5: Persistente Speicherung
- Als Benutzer möchte ich, dass meine Konfiguration auch nach Neustart erhalten bleibt
- Akzeptanzkriterien:
  - Konfiguration wird im NVS gespeichert
  - Zuverlässiges Laden der gespeicherten Konfiguration
  - Fehlerbehandlung bei Speicherproblemen 