# Button Implementation

## Overview
Implementation of physical button handling for the ESP32-KeyMaster device.

## Features
- Debouncing of button inputs
- Long press detection
- Pull-up resistor configuration
- Button state management

## Technical Details
- Button A: GPIO 1
- Button B: GPIO 4
- Internal pull-up resistors enabled
- Debounce time: 30ms (non-blocking)
- Long press duration: 5000ms

## Implementation
```cpp
// Button pin definitions
#define BUTTON_A_PIN 1
#define BUTTON_B_PIN 4

// Debounce configuration
const unsigned long debounceDuration = 30;
const unsigned long longPressDuration = 5000;
```

## Usage
- Short press: Triggers configured key combination
- Long press (Button B): Enters configuration mode
- LED feedback on button press 
