# Web UI Design

## Overview
Design and implementation of the user interface for the ESP32-KeyMaster web configuration.

## Features
- Responsive design
- Modern UI elements
- Intuitive layout
- Mobile-friendly interface
- Consistent styling

## Design Elements
- Color scheme
  - Primary: #0078d7 (Blue)
  - Secondary: #6c757d (Gray)
  - Background: #f7f7f7 (Light Gray)
  - Text: #333333 (Dark Gray)
  - Success: #28a745 (Green)
  - Warning: #ffc107 (Yellow)
  - Error: #dc3545 (Red)

## UI Components
- Input fields
  - Key code input
  - Color picker
  - Hex color input
- Checkboxes
  - Modifier keys
- Buttons
  - Save configuration
  - Exit configuration
  - Back to main
- Navigation
  - Debug link
  - Help link
  - About link

## Responsive Design
- Mobile-first approach
- Flexible layouts
- Adaptive components
- Touch-friendly elements

## CSS Implementation
```css
body {
  font-family: sans-serif;
  margin: 0;
  padding: 0;
  background: #f7f7f7;
}

.container {
  max-width: 420px;
  margin: 20px auto;
  background: #fff;
  border-radius: 10px;
  box-shadow: 0 2px 8px #0001;
  padding: 20px;
}

/* Responsive adjustments */
@media (max-width: 500px) {
  .container {
    padding: 15px;
    margin: 10px;
  }
}
```

## User Experience
- Clear navigation
- Intuitive controls
- Immediate feedback
- Error prevention
- Helpful tooltips
- Consistent behavior 