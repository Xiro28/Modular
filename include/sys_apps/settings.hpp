#pragma once
#include "os/interfaces/application_interface.hpp"

#include <vector>
#include <os/modules/toastmessages.hpp>


// Stati interni dell'App Settings
enum SettingsState {
    PAGE_MAIN,
    PAGE_WIFI_SCAN,
    PAGE_WIFI_KEYBOARD,
    PAGE_DAAS,
    PAGE_STATS
};

class SettingsApp : public Application {
private:
    SettingsState currentState = PAGE_MAIN;
    bool currentSessionSaved = true;


    // --- State Data ---
    bool btEnabled = false;       
    uint64_t currentDIN = 123456789; 
    String targetSSID = "";       
    String inputBuffer = "";      
    
    unsigned long lastStatsUpdate = 0;

    // Layout Constants
    const int ITEM_H = 50;

    // --- GRAPHIC HELPERS ---
    
    void drawTile(int index, const char* label, const char* status, uint16_t accentColor) {
        // Grid Logic: 2 Columns. 
        // Index 0: Top Left, 1: Top Right, 2: Bottom Left, 3: Bottom Right
        int col = index % 2;
        int row = index / 2;

        int margin = 10;
        int w = (hw->tft.width() - (margin * 3)) / 2; // Calculate width dynamically
        int h = 90; // Fixed height for tiles
        int x = margin + (col * (w + margin));
        int y = 60 + (row * (h + margin)); // Start Y at 60

        // 1. Shadow
        hw->tft.fillRoundRect(x, y+4, w, h, 8, theme->PANEL_SHADOW);
        // 2. Main Body
        hw->tft.fillRoundRect(x, y, w, h, 8, theme->PANEL_BG);

        // 3. Status Dot/Bar
        hw->tft.fillRoundRect(x + 10, y + 10, 30, 6, 3, accentColor);

        // 4. Label (Bottom of tile)
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
        hw->tft.setTextDatum(textdatum_t::bottom_left);
        hw->tft.drawString(label, x + 10, y + h - 10);

        // 5. Status Text (Middle)
        hw->tft.setTextColor(theme->TEXT_MUTED, theme->PANEL_BG);
        hw->tft.setTextDatum(textdatum_t::top_left);
        hw->tft.drawString(status, x + 10, y + 25);
    }

    void drawListItem(int index, const char* label, const char* value, bool isToggle = false, bool toggleState = false) {
        int y = 50 + (index * ITEM_H);
        int w = hw->tft.width();
        
        // Background
        hw->tft.fillRect(5, y, w - 10, ITEM_H - 5, theme->PANEL_BG);
        hw->tft.drawRect(5, y, w - 10, ITEM_H - 5, theme->BORDER_COLOR);
        
        // Label
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
        hw->tft.setTextDatum(textdatum_t::middle_left);
        hw->tft.drawString(label, 15, y + (ITEM_H/2) - 2);

        // Value or Toggle
        if (isToggle) {
            int toggleX = w - 50; 
            uint16_t tColor = toggleState ? theme->ACCENT_PRIMARY : theme->TEXT_MUTED;
            hw->tft.fillRoundRect(toggleX, y + 10, 35, 20, 10, tColor);
            hw->tft.fillCircle(toggleState ? toggleX + 25 : toggleX + 10, y + 20, 8, theme->TEXT_MAIN);
        } else {
            hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
            hw->tft.setTextDatum(textdatum_t::middle_right);
            hw->tft.drawString(value, w - 15, y + (ITEM_H/2) - 2);
        }
    }

    // Modern Header (No block background)
    void drawHeader(const char* title, bool showBack = true, bool clearBg = true) {
        if (clearBg) {
            hw->tft.fillScreen(theme->BG_COLOR);
        
            // Large Modern Title
            hw->tft.setTextColor(theme->TEXT_MAIN, theme->BG_COLOR);
            hw->tft.setTextDatum(textdatum_t::middle_left);
            // Assuming you have a larger font, if not, standard is fine
            hw->tft.drawString(title, showBack ? 30 : 15, 30);
            
            // Divider
            hw->tft.drawLine(15, 50, 60, 50, theme->ACCENT_PRIMARY); // Small accent line
        }

        if (showBack) {
            hw->tft.drawString("<", 10, 30);
        }
    }

    void drawButton(int x, int y, int w, int h, const char* label, uint16_t bgCol, uint16_t txtCol) {
        // Shadow
        hw->tft.fillRoundRect(x, y + 4, w, h, 8, theme->PANEL_SHADOW);
        // Body
        hw->tft.fillRoundRect(x, y, w, h, 8, bgCol);
        // Text
        hw->tft.setTextColor(txtCol, bgCol);
        hw->tft.setTextDatum(textdatum_t::middle_center);
        hw->tft.drawString(label, x + w/2, y + h/2);
        hw->tft.setTextDatum(textdatum_t::top_left);
    }

public:
    SettingsApp() : Application(1) {} // ID 1
    
    void onStart() override {
        currentState = PAGE_MAIN;
        needsRedraw = true;
    }

    void onUpdate() override {
        // --- Auto-Save Logic ---
        // If we are connected successfully but haven't saved this session yet
        if (WiFi.status() == WL_CONNECTED && !currentSessionSaved) {
            currentSessionSaved = system->getHW()->saveCurrentWifi();
            needsRedraw = true; // Refresh UI to show connection status
        }

        switch (currentState) {
            case PAGE_MAIN:
                if (needsRedraw) { drawMainPage(); needsRedraw = false; }
                handleMainTouch();
                break;
            case PAGE_WIFI_SCAN:
                if (needsRedraw) { drawWifiPage(); needsRedraw = false; }
                handleWifiTouch();
                break;
            case PAGE_WIFI_KEYBOARD:
                if (needsRedraw) { 
                    system->getKeyboard()->begin("Enter Wi-Fi Password:");
                    needsRedraw = false; 
                }
                
                if (system->getKeyboard()->isDone()) {
                    if (!system->getKeyboard()->wasCancelled()) {
                        String password = system->getKeyboard()->getResult();
                        
                        // Initiate connection
                        WiFi.disconnect(); // Start fresh each time
                        WiFi.begin(targetSSID.c_str(), password.c_str());

                        
                        // Reset save flag so onUpdate knows to save when connection succeeds
                        currentSessionSaved = false; 
                    }
                    currentState = PAGE_MAIN;
                    needsRedraw = true;
                }

                system->getKeyboard()->update();
                break;
            case PAGE_DAAS:
                if (needsRedraw) { drawDaasPage(); needsRedraw = false; }
                handleDaasTouch();
                break;
            case PAGE_STATS:
                if (millis() - lastStatsUpdate > 1000 || needsRedraw) {
                    drawStatsPage();
                    needsRedraw = false;
                    lastStatsUpdate = millis();
                }
                handleStatsTouch();
                break;
        }
    }

    void onExit() override { }
    void onDraw() override { }

    void drawMainPage() {
        drawHeader("DASHBOARD");

        // 1. Wi-Fi Tile
        bool wifiCon = (WiFi.status() == WL_CONNECTED);
        drawTile(0, "Wi-Fi", wifiCon ? "Online" : "Offline", wifiCon ? theme->ACCENT_PRIMARY : theme->TEXT_MUTED);

        // 2. Bluetooth Tile
        drawTile(1, "Bluetooth", btEnabled ? "Active" : "Disabled", btEnabled ? theme->ACCENT_PRIMARY : theme->TEXT_MUTED);

        // 3. DaaS Tile
        drawTile(2, "DaaS Cloud", system->daasNetworkConnected ? "Connected" : "Configure", system->daasNetworkConnected ? theme->ACCENT_PRIMARY : theme->ACCENT_WARN);

        // 4. Stats Tile
        drawTile(3, "System", "View Stats", theme->ACCENT_ALERT);
    }

    void drawWifiPage() {
        drawHeader("SCANNING...");
        int n = WiFi.scanNetworks();
        hw->tft.fillScreen(theme->BG_COLOR);
        drawHeader("WI-FI");
        
        for (int i = 0; i < n && i < 5; ++i) {
             String ssid = WiFi.SSID(i);
             // Truncate long SSIDs
             if(ssid.length() > 10) ssid = ssid.substring(0, 9) + ".";
             String label = ssid + " (" + String(WiFi.RSSI(i)) + ")";
             drawListItem(i, label.c_str(), ">");
        }
        if (n == 0) hw->tft.drawString("No AP Found", 20, 60);
    }

    void drawDaasPage() {
        drawHeader("DaaS CONFIG", true); // showBack = true
        int w = hw->tft.width();

        // --- 1. CONNECTION STATUS CARD ---
        int cardY = 60;
        int cardH = 90;
        
        hw->tft.fillRoundRect(10, cardY, w - 20, cardH, 8, theme->PANEL_BG);
        hw->tft.drawRoundRect(10, cardY, w - 20, cardH, 8, theme->BORDER_COLOR);

        // Determine active technology
        bool wifiReady = (WiFi.status() == WL_CONNECTED);
        bool btReady = btEnabled; 

        // Status Indicator Circle
        uint16_t statusColor = (wifiReady || btReady) ? theme->ACCENT_PRIMARY : theme->ACCENT_ALERT;
        hw->tft.fillCircle(30, cardY + 25, 6, statusColor);

        hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
        hw->tft.setTextDatum(textdatum_t::middle_left);
        hw->tft.drawString(wifiReady ? "Wi-Fi Active" : (btReady ? "Bluetooth Active" : "No Connection"), 45, cardY + 25);

        // URI Display
        String uri = "Unavailable";
        if (wifiReady) uri = WiFi.localIP().toString() + ":9909";
        else if (btReady) uri = "BT: " + String(currentDIN); // Example BT URI

        hw->tft.setTextColor(theme->TEXT_MUTED, theme->PANEL_BG);
        hw->tft.drawString(uri, 30, cardY + 55);


        if (wifiReady || btReady) {
            drawButton(10, 170, w - 20, 45, "ENABLE DRIVER", theme->ACCENT_PRIMARY, theme->TEXT_MAIN);
        } else {
            // Disabled state visual
             drawButton(10, 170, w - 20, 45, "No Link Available", theme->PANEL_SHADOW, theme->TEXT_MUTED);
        }

        // --- 3. NETWORK MANAGEMENT ---
        int bottomY = 230;
        int btnW = (w - 30) / 2;
        
        drawButton(10, bottomY, btnW, 45, "UNBIND", theme->ACCENT_ALERT, theme->TEXT_MAIN);
        drawButton(20 + btnW, bottomY, btnW, 45, "DISCOVER", theme->ACCENT_WARN, theme->TEXT_MAIN); // Changed to WARN for contrast
    }

    void drawStatsPage() {
        drawHeader("SYSTEM STATS", true, needsRedraw); // Title with Back button
        int w = hw->tft.width();
        int m = 10; // Margin

        // --- SECTION 1: TRAFFIC DASHBOARD (Card) ---
        int yTraffic = 60;
        int hTraffic = 80;
        
        // Card Background
        hw->tft.fillRoundRect(m, yTraffic, w - 2*m, hTraffic, 8, theme->PANEL_BG);
        hw->tft.drawRoundRect(m, yTraffic, w - 2*m, hTraffic, 8, theme->BORDER_COLOR); // Subtle border

        // Section Label
        hw->tft.setTextColor(theme->ACCENT_PRIMARY, theme->PANEL_BG);
        hw->tft.setTextDatum(textdatum_t::top_left);
        hw->tft.drawString("MESSAGE TRAFFIC", m + 10, yTraffic + 5);

        // 3 Columns: Sent | Recv | Routed
        int colW = (w - 2*m) / 3;
        
        auto drawStatItem = [&](int idx, const char* label, uint32_t val, uint16_t color) {
            int x = m + (idx * colW);
            int center = x + (colW / 2);
            int yVal = yTraffic + 30;
            
            // Value (Big Number)
            hw->tft.setTextColor(color, theme->PANEL_BG);
            hw->tft.setTextDatum(textdatum_t::top_center);
            hw->tft.drawString(String(val), center, yVal);
            
            // Label (Small Text)
            hw->tft.setTextColor(theme->TEXT_MUTED, theme->PANEL_BG);
            hw->tft.drawString(label, center, yVal + 20);
            
            // Vertical Divider line (except for last item)
            if(idx < 2) hw->tft.drawFastVLine(x + colW, yVal, 30, theme->BORDER_COLOR);
        };

        drawStatItem(0, "SENT", system->getNode()->getSystemStatistics(_cor_dme_sended), theme->TEXT_MAIN);
        drawStatItem(1, "RECV", system->getNode()->getSystemStatistics(_cor_dme_received), theme->ACCENT_PRIMARY);
        drawStatItem(2, "ROUT", system->getNode()->getSystemStatistics(_cor_dme_routed), theme->ACCENT_WARN);


        // --- SECTION 2: SYSTEM HEALTH (2x2 Grid) ---
        int ySys = 150;
        int hSys = 90;
        
        hw->tft.fillRoundRect(m, ySys, w - 2*m, hSys, 8, theme->PANEL_BG);
        
        auto drawSysRow = [&](int row, const char* l1, String v1, const char* l2, String v2) {
            int y = ySys + 20 + (row * 35);
            int mid = w / 2;
            
            // Left Column
            hw->tft.setTextColor(theme->TEXT_MUTED, theme->PANEL_BG);
            hw->tft.setTextDatum(textdatum_t::top_left);
            hw->tft.drawString(l1, m + 15, y);
            

            
            if (row == 1) {
                // clear area for UP text to avoid artifacts
                hw->tft.drawRect(mid - 5, y, (w/2) - m - 10, 20, theme->PANEL_BG);
            }

            hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
            hw->tft.setTextDatum(textdatum_t::top_right);
            hw->tft.drawString(v1, mid - 5, y);
            

            // Right Column
            hw->tft.setTextColor(theme->TEXT_MUTED, theme->PANEL_BG);
            hw->tft.setTextDatum(textdatum_t::top_left);
            hw->tft.drawString(l2, mid + 10, y);
            
            hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
            hw->tft.setTextDatum(textdatum_t::top_right);
            hw->tft.drawString(v2, w - m - 15, y);
        };

        String heap = String(ESP.getFreeHeap()/1024) + "k";
        String blk = String(ESP.getMaxAllocHeap()/1024) + "k";
        String up = String(millis()/1000) + "s";
        
        drawSysRow(0, "Free:", heap, "Max:", blk);
        drawSysRow(1, "Up:", up, "Ver:", system->getNode()->getVersion());
        
        // Vertical Split Line
        hw->tft.drawFastVLine(w/2, ySys + 15, hSys - 30, theme->BORDER_COLOR);


        // --- SECTION 3: SIGNAL VISUALIZER ---
        int ySig = 250;
        int hSig = 50;
        hw->tft.fillRoundRect(m, ySig, w - 2*m, hSig, 8, theme->PANEL_BG);
        
        // Calculate Signal Bars (0 to 4)
        int rssi = WiFi.RSSI();
        int bars = 0;
        if (WiFi.status() == WL_CONNECTED) {
            if (rssi > -55) bars = 4;
            else if (rssi > -65) bars = 3;
            else if (rssi > -75) bars = 2;
            else if (rssi > -85) bars = 1;
        }

        // Label
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
        hw->tft.setTextDatum(textdatum_t::middle_left);
        hw->tft.drawString("Wi-Fi Signal", m + 15, ySig + hSig/2);
        
        // Draw Bars
        int barStart = w - m - 70;
        int barBottom = ySig + 35;
        
        for(int i=0; i<4; i++) {
            int h = 8 + (i*6); // Ascending height
            bool active = (i < bars);
            uint16_t color = active ? (bars < 2 ? theme->ACCENT_ALERT : theme->ACCENT_PRIMARY) : theme->PANEL_SHADOW;
            hw->tft.fillRect(barStart + (i*12), barBottom - h, 8, h, color);
        }
        
        // dBm Text (Tiny)
        hw->tft.setTextColor(theme->TEXT_MUTED, theme->PANEL_BG);
        hw->tft.setTextDatum(textdatum_t::top_right);
        hw->tft.drawString(String(rssi) + "dBm", w - m - 10, ySig + 10);
    }

    // --- TOUCH LOGIC ---

    void handleMainTouch() {
        if (!hw->isTouching) return;
        delay(200);

        int tx = hw->touchX;
        int ty = hw->touchY;
        
        // Header / Back check (if needed later)
        if (ty < 50 && tx < 50) { system->launchApp((u8_t)0); return; }

        // Grid Hit Detection
        // Approx Y range: Row 1 (60-150), Row 2 (160-250)
        // Approx X range: Col 1 (0-120), Col 2 (120-240)
        
        int row = -1;
        int col = -1;

        if (ty > 60 && ty < 150) row = 0;
        else if (ty > 160 && ty < 250) row = 1;

        if (tx < 120) col = 0;
        else col = 1;

        if (row != -1 && col != -1) {
            int index = (row * 2) + col;
            
            if (index == 0) { currentState = PAGE_WIFI_SCAN; needsRedraw = true; }
            if (index == 1) { btEnabled = !btEnabled; needsRedraw = true; }
            if (index == 2) { currentState = PAGE_DAAS; needsRedraw = true; }
            if (index == 3) { currentState = PAGE_STATS; needsRedraw = true; }
        }
    }

    void handleWifiTouch() {
        if (!hw->isTouching) return;
        delay(200);

        if (hw->isTouchInRect(0, 0, 50, 40)) {
            currentState = PAGE_MAIN; needsRedraw = true; return;
        }

        int index = (hw->touchY - 50) / ITEM_H;
        if (index >= 0) {
            targetSSID = WiFi.SSID(index);
            inputBuffer = "";
            currentState = PAGE_WIFI_KEYBOARD;
            needsRedraw = true;
        }
    }

    void handleDaasTouch() {
        if (!hw->isTouching) return;
        delay(200);
        int w = hw->tft.width();

        // Back Button (Top Left)
        if (hw->touchY < 50 && hw->touchX < 50) {
            currentState = PAGE_MAIN; needsRedraw = true; return;
        }

        bool wifiReady = (WiFi.status() == WL_CONNECTED);
        bool btReady = btEnabled;

        // ENABLE DRIVER BUTTON (Y: 170, H: 45)
        if (hw->isTouchInRect(10, 170, w - 20, 45)) {
            if (wifiReady) {
                String uri = WiFi.localIP().toString() + ":9909";
                system->getNode()->enableDriver(_LINK_INET4, uri.c_str());
                ToastManager::getInstance()->show("Driver Enabled (Wi-Fi)", TOAST_INFO, 2500);
                // Optional: Show visual feedback
            } 
            else if (btReady) {
                // Assuming you have a specific driver constant for BT, e.g., _LINK_BLUETOOTH
                // And a specific URI format
                String btUri = String(currentDIN); 
                system->getNode()->enableDriver(_LINK_BT, btUri.c_str()); 
            }
        }

        // UNBIND (Bottom Left)
        int bottomY = 230;
        int btnW = (w - 30) / 2;
        if (hw->isTouchInRect(10, bottomY, btnW, 45)) {
            system->getNode()->unbindNetwork();
            system->daasNetworkConnected = false;
            needsRedraw = true;
            ToastManager::getInstance()->show("Network Unbound", TOAST_INFO);
        }

        // DISCOVER (Bottom Right)
        if (hw->isTouchInRect(20 + btnW, bottomY, btnW, 45)) {
            system->getNode()->discovery();
            ToastManager::getInstance()->show("Discovery Started", TOAST_INFO, 1000);
        }
    }

    void handleStatsTouch() {
        if (!hw->isTouching) return;
        delay(200);
        if (hw->isTouchInRect(0, 0, 100, 40)) { 
            currentState = PAGE_MAIN; needsRedraw = true;
        }
    }
};