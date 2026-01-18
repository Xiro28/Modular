#pragma once
#include <Arduino.h>

enum AppType {
    APP_INTERNAL, // Hardcoded (Settings, Terminal)
    APP_EXTERNAL  // Loaded from SD (.bin or script)
};

struct AppShortcut {
    String name;       // Display Name (e.g., "Doom")
    String iconPath;   // Path to .bmp/.jpg (optional, future use)
    uint16_t color;    // Fallback color if no icon image
    AppType type;
    String execPath;   // Path to executable (e.g., "/sd/doom.bin")
};