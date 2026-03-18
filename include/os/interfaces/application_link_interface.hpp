#pragma once

#include <string>

enum AppType {
    APP_INTERNAL, // Hardcoded (Settings, Terminal)
    APP_EXTERNAL  // Loaded from SD (.bin or script)
};

struct AppShortcut {
    std::string name;       // Display Name (e.g., "Doom")
    std::string iconPath;   // Path to .bmp/.jpg (optional, future use)
    uint16_t color;    // Fallback color if no icon image
    AppType type;
    std::string execPath;   // Path to executable (e.g., "/sd/doom.bin")
    uint8_t id;          // Unique App ID (assigned at registration)
};