# LED Implementation

## Overview
Implementation of RGB LED feedback using WS2812B/NeoPixel for the ESP32-KeyMaster device.

## Features
- RGB color control
- Configurable colors per button
- Visual feedback for button states
- Configuration mode indication

## Technical Details
- LED Type: WS2812B/NeoPixel
- GPIO: 48 (onboard WS2812 on ESP32-S3 Super Mini)
- Number of LEDs: 1
- Brightness: 20 (0-255)
 - Hinweis: GPIO 48 teilt sich die Leitung mit der roten Power-LED

## Implementation
```cpp
// LED configuration
#define LED_PIN 48
#define NUMPIXELS 1
#define BRIGHTNESS 20

// LED colors for different states
#define COLOR_CONFIG 255, 150, 0  // Orange for config mode
#define COLOR_SUCCESS 0, 20, 0    // Green for success
```

## Usage
- Button press: Shows configured color
- Configuration mode: Orange
- Success indication: Green
- Error indication: Red 
