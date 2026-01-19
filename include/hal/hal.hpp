#pragma once

#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <Preferences.h>

// Make sure you have the LovyanGFX library installed
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// NOTE: Ensure this file exists in your project. 
// If you don't have it, comment it out and change the font in init().
#include "ESP32_SPI_9341.h" 

// --- PIN DEFINITIONS FOR CYD ---
#define SD_CS_PIN 5

class LGFX_CYD : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_SPI       _bus_instance;
    lgfx::Touch_XPT2046 _touch_instance;
    lgfx::Light_PWM     _light_instance;

public:
    LGFX_CYD(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = HSPI_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 55000000;
            cfg.freq_read  = 16000000;
            cfg.spi_3wire  = true;
            cfg.use_lock   = true;
            cfg.dma_channel = 1;
            cfg.pin_sclk = 14;
            cfg.pin_mosi = 13;
            cfg.pin_miso = 12;
            cfg.pin_dc   = 2;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = 15;
            cfg.pin_rst          = -1;
            cfg.pin_busy         = -1;
            cfg.panel_width      = 240;
            cfg.panel_height     = 320;
            cfg.offset_rotation  = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits  = 1;
            cfg.readable         = true;
            cfg.invert           = false;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = false;
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _light_instance.config();
            cfg.pin_bl      = 21;
            cfg.invert      = false;
            cfg.freq        = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        { 
            auto cfg = _touch_instance.config();
            
            cfg.x_min      = 240;  
            cfg.x_max      = 3800; 
            cfg.y_min      = 240;  
            cfg.y_max      = 3800; 
            
            cfg.pin_int    = -1; 
            
            cfg.bus_shared = false;
            cfg.offset_rotation = 0;

            cfg.spi_host = -1; 
            cfg.pin_sclk = 25;
            cfg.pin_mosi = 32;
            cfg.pin_miso = 39;
            cfg.pin_cs   = 33;
            cfg.freq     = 20000000;

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

// --- HARDWARE MANAGER CLASS ---
class HardwareManager {
public:
    LGFX_CYD tft;
    Preferences prefs;
    bool sdAvailable = false;
    
    // Input State
    int touchX = 0; 
    int touchY = 0;
    bool isTouching = false;

    void init() {
        Serial.begin(115200);

        // 1. Init Display
        tft.begin();
        tft.setRotation(0); // 0 = Portrait, 1 = Landscape (Adjust if touch coordinates are flipped)
        
        // Ensure this font exists, otherwise use &fonts::Font4
        tft.setFont(&fonts::efontCN_14); 

        // 2. Init SD Card (VSPI)
        SPI.begin(18, 19, 23);
    
        // Sicurezza extra: Pin CS Touch alto (spento) per non disturbare la SD
        pinMode(33, OUTPUT);
        digitalWrite(33, HIGH);

        if (SD.begin(SD_CS_PIN, SPI, 10000000)) {
            sdAvailable = true;
            Serial.println("SYSTEM: SD Card Mounted");
        } else {
            sdAvailable = false;
            Serial.println("SYSTEM: SD Card Missing or Fail - Running Safe Mode");
        }
        
        // 3. Init WiFi / Prefs
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(); // Start fresh
        loadSavedWifi();
    }

    // Update Input State
    void updateInput() {
        uint16_t rawX, rawY;
        // getTouch returns true if screen is pressed
        isTouching = tft.getTouch(&rawX, &rawY);
        
        if (isTouching) {
            // Mapping Logic (Adjust based on Rotation 0)
            // Since x_min/max are 0-4095, getTouch returns mapped pixel coordinates
            // based on Lovyan's internal calculation.
            
            touchY = 320 - rawY; 
            touchX = rawX;
            
            // Debugging (Uncomment to see values in Serial Monitor)
            // Serial.printf("Touch: X=%d Y=%d\n", touchX, touchY);
        }
    }

    // Helper for Rect collision
    bool isTouchInRect(int x, int y, int w, int h) {
        return (isTouching && touchX >= x && touchX <= x + w && touchY >= y && touchY <= y + h);
    }

    void resetScreen(uint16_t bgColor) {
        tft.fillScreen(bgColor);
        tft.setCursor(0, 0);
        tft.setTextDatum(textdatum_t::top_left);
        tft.setFont(&fonts::efontCN_14);
        tft.setTextSize(1);
    }

    bool loadSavedWifi() {
        prefs.begin("wifi_conf", true); // Read-only
        String s = prefs.getString("ssid", "");
        String p = prefs.getString("pass", "");
        prefs.end();

        if (s.length() > 0) {
            WiFi.begin(s.c_str(), p.c_str());
            return true;
        }
        return false;
    }

    bool saveCurrentWifi() {
        if (WiFi.status() == WL_CONNECTED) {
            prefs.begin("wifi_conf", false); // Read/Write
            prefs.putString("ssid", WiFi.SSID());
            prefs.putString("pass", WiFi.psk());
            prefs.end();
            return true;
        }
        return false;
    }
};