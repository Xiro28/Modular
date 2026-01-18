#pragma once
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <Preferences.h>

#include "ESP32_SPI_9341.h"

class HardwareManager
{
public:
    LGFX tft;
    Preferences prefs;
    bool sdAvailable = false;
    int touchX = 0, touchY = 0;
    bool isTouching = false;

    void init()
    {
        Serial.begin(115200);

        // 1. Init Display
        tft.begin();
        tft.setRotation(0); // Adjust as needed
        tft.setFont(&fonts::efontCN_14);

        // 2. Init Storage (Fault Tolerant)
        // Try to mount SD, if fails, system continues
        if (SD.begin())
        {
            sdAvailable = true;
            Serial.println("SYSTEM: SD Card Mounted");
        }
        else
        {
            sdAvailable = false;
            Serial.println("SYSTEM: SD Card Missing or Fail - Running Safe Mode");
        }

        
        WiFi.mode(WIFI_STA);
        WiFi.disconnect(); // Start fresh each time
        loadSavedWifi();
    }

    // Update Input State
    void updateInput()
    {
        uint16_t rawX, rawY;
        isTouching = tft.getTouch(&rawX, &rawY);
        if (isTouching)
        {
            // Map your specific touch coordinates here
            touchY = map(rawY, 125, 3727, 320, 0);
            touchX = map(rawX, 142, 3871, 0, 240);
        }
    }

    // Helper for Rect collision
    bool isTouchInRect(int x, int y, int w, int h)
    {
        return (isTouching && touchX >= x && touchX <= x + w && touchY >= y && touchY <= y + h);
    }

    void resetScreen(uint16_t bgColor)
    {
        tft.fillScreen(bgColor);

        tft.setCursor(0, 0);
        tft.setTextDatum(textdatum_t::top_left);

        tft.setFont(&fonts::efontCN_14);
        tft.setTextSize(1);
    }

    bool loadSavedWifi()
    {
        prefs.begin("wifi_conf", true); // Open in read-only mode
        String s = prefs.getString("ssid", "");
        String p = prefs.getString("pass", "");
        prefs.end();

        if (s.length() > 0)
        {
            WiFi.begin(s.c_str(), p.c_str());
            return true;
        }

        return false;
    }

    bool saveCurrentWifi()
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            prefs.begin("wifi_conf", false); // Open in read/write mode
            prefs.putString("ssid", WiFi.SSID());
            prefs.putString("pass", WiFi.psk());
            prefs.end();
            return true;
        }

        return false;
    }
};