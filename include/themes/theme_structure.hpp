#pragma once
#include <Arduino.h>
#include <LovyanGFX.hpp>

/*struct ThemePalette {
    uint16_t BG_COLOR;
    uint16_t PANEL_BG;
    uint16_t HEADER_BG;
    uint16_t TEXT_MAIN;
    uint16_t TEXT_MUTED;
    uint16_t TEXT_INFO;
    uint16_t ACCENT_PRIMARY;
    uint16_t ACCENT_WARN;
    uint16_t ACCENT_ALERT;
    uint16_t BORDER_COLOR;
    uint16_t ACTIVE_COLOR;
};

const ThemePalette DEFAULT_THEME = {
    0x0000, 0x1084, 0x18E3, 0xFFFF, 0x8410, 0x07E0, 
    0x901F, 0xE720, 0xF800, 0x4A69, 0x07FF
};*/

// Add these colors to your ThemePalette struct or definition
// Converted to RGB565 for TFT
struct ThemePalette {
    uint16_t BG_COLOR      ; // Deep Dark Slate (Background)
    uint16_t PANEL_BG      ; // Lighter Slate (Key/Tile surface)
    uint16_t PANEL_SHADOW  ; // Dark Shadow (For depth)
    uint16_t TEXT_MAIN     ; // White
    uint16_t TEXT_MUTED    ; // Gray
    uint16_t ACCENT_PRIMARY; // Neon Blue (Active)
    uint16_t ACCENT_WARN   ; // Soft Orange
    uint16_t ACCENT_ALERT  ; // Red
    uint16_t BORDER_COLOR  ; // Subtle border
    uint16_t HEADER_BG     ; // Same as BG for "Clean" look
};

extern ThemePalette DEFAULT_THEME;