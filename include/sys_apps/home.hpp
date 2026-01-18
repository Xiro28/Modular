class HomeApp : public Application {
private:
    // Grid Configuration
    const int COLS = 3;
    const int ROWS = 3; 
    const int ICON_SIZE = 60;
    const int GAP = 15;
    // Centering calculation: (240 - (3*60 + 2*15)) / 2 = 15
    const int START_X = 15; 
    const int START_Y = 60;

    // Helper: Draw a single app icon with "Depth"
    void drawAppIcon(int col, int row, const char* label, uint16_t color, bool isAddBtn = false) {
        int x = START_X + (col * (ICON_SIZE + GAP));
        int y = START_Y + (row * (ICON_SIZE + 35)); // More vertical space for text

        // 1. Icon Shadow (Offset)
        hw->tft.fillRoundRect(x, y + 4, ICON_SIZE, ICON_SIZE, 14, theme->PANEL_SHADOW);

        // 2. Icon Body (Squircle)
        hw->tft.fillRoundRect(x, y, ICON_SIZE, ICON_SIZE, 14, color);

        // 3. Subtle Inner Border (Top/Left Highlight)
        // hw->tft.drawRoundRect(x, y, ICON_SIZE, ICON_SIZE, 14, 0xFFFF); // Optional Gloss

        // 4. Icon Symbol
        hw->tft.setTextColor(isAddBtn ? theme->TEXT_MUTED : theme->TEXT_MAIN);
        hw->tft.setTextDatum(textdatum_t::middle_center);
        
        if (isAddBtn) {
            hw->tft.setFont(&fonts::efontCN_24);
            hw->tft.drawString("+", x + ICON_SIZE/2, y + ICON_SIZE/2 - 2);
            hw->tft.setFont(&fonts::efontCN_14);
        } else {
            // Draw first letter as logo
            String initial = String(label).substring(0, 1);
            initial.toUpperCase();
            hw->tft.setFont(&fonts::efontCN_24);
            hw->tft.drawString(initial, x + ICON_SIZE/2, y + ICON_SIZE/2);
            hw->tft.setFont(&fonts::efontCN_14);
        }

        // 5. Label (Below icon)
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->BG_COLOR);
        hw->tft.setTextDatum(textdatum_t::top_center);
        hw->tft.drawString(label, x + ICON_SIZE/2, y + ICON_SIZE + 8);
        hw->tft.setTextDatum(textdatum_t::top_left); // Reset
    }

    void drawStatusBar() {
        int w = hw->tft.width();
        int h = 30;
        
        // Background (Slightly lighter than main BG to differentiate)
        hw->tft.fillRect(0, 0, w, h, theme->HEADER_BG);
        hw->tft.drawFastHLine(0, h, w, theme->PANEL_SHADOW); // Separator line

        // --- LEFT: TIME (Simulated) ---
        unsigned long upSeconds = system->getNode()->getSyncedTimestamp() / 1000;
        int mins = (upSeconds / 60) % 60;
        int hrs = (upSeconds / 3600) % 24;
        char timeStr[6];
        sprintf(timeStr, "%02d:%02d", hrs, mins);
        
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->HEADER_BG);
        hw->tft.setTextDatum(textdatum_t::middle_left);
        hw->tft.drawString(timeStr, 8, h/2);

        // --- RIGHT: ICONS ---
        int xPos = w - 10;
        int yCenter = h/2;

        // 1. BATTERY (Simulated visual)
        hw->tft.drawRect(xPos - 20, yCenter - 5, 18, 10, theme->TEXT_MUTED);
        hw->tft.fillRect(xPos - 18, yCenter - 3, 10, 6, theme->TEXT_MAIN); // 60% charge
        hw->tft.fillRect(xPos - 2, yCenter - 2, 2, 4, theme->TEXT_MUTED); // Nub
        xPos -= 28;

        // 2. WI-FI SIGNAL
        if (WiFi.status() == WL_CONNECTED) {
            int rssi = WiFi.RSSI();
            // Draw 3 bars
            for(int i=0; i<3; i++) {
                int barH = 4 + (i*3);
                bool active = (i==0) || (i==1 && rssi > -80) || (i==2 && rssi > -60);
                hw->tft.fillRect(xPos - 10 + (i*4), yCenter + 5 - barH, 3, barH, active ? theme->TEXT_MAIN : theme->PANEL_SHADOW);
            }
        } else {
             hw->tft.setTextColor(theme->TEXT_MUTED, theme->HEADER_BG);
             hw->tft.setTextDatum(textdatum_t::middle_right);
             hw->tft.drawString("x", xPos, yCenter);
        }
        xPos -= 15;

        // 3. BLUETOOTH (Only if enabled)
        // Assuming you have access to btEnabled from somewhere, or check a global
        // For now, placeholder:
        // hw->tft.drawString("B", xPos, yCenter);
    }
    
    public:
    HomeApp() : Application(0) {}
    
    void onStart() override {
        needsRedraw = true;
    }

    void onUpdate() override {
        if (needsRedraw) {
            drawGrid();
            needsRedraw = false;
        }
        handleTouch();
    }

    void onDraw() override { }
    void onExit() override { }

    void drawGrid() {
        hw->tft.fillScreen(theme->BG_COLOR);
        
        drawStatusBar();
        
        // Draw Installed Apps
        auto& apps = system->registry.getApps();
        int count = 0;
        
        for (const auto& app : apps) {
            int col = count % COLS;
            int row = count / COLS;
            if (row >= ROWS) break; 
            
            drawAppIcon(col, row, app.name.c_str(), app.color);
            count++;
        }

        // Draw "Add App" Button
        int col = count % COLS;
        int row = count / COLS;
        if (row < ROWS) {
            drawAppIcon(col, row, "Add", theme->PANEL_BG, true);
        }
    }

    void highlightApp(int index) {
        int col = index % COLS;
        int row = index / COLS;
        
        int x = START_X + (col * (ICON_SIZE + GAP));
        int y = START_Y + (row * (ICON_SIZE + 35));

        // Draw a bright border to indicate selection
        hw->tft.drawRoundRect(x-2, y-2, ICON_SIZE+4, ICON_SIZE+4, 16, theme->ACCENT_PRIMARY);
        delay(100); 
        // Redraw the background part to erase border
        // (Full redraw is safer but slower, this is a quick patch)
        hw->tft.drawRoundRect(x-2, y-2, ICON_SIZE+4, ICON_SIZE+4, 16, theme->BG_COLOR);
    }

    void handleTouch() {
        if (!hw->isTouching) return;
        delay(150); 
        
        int x = hw->touchX;
        int y = hw->touchY;

        // Ignore clicks on Status Bar
        if (y < 40) return;

        auto& apps = system->registry.getApps();
        int totalItems = apps.size() + 1;

        for (int i = 0; i < totalItems; i++) {
            int col = i % COLS;
            int row = i / COLS;
            
            int iconX = START_X + (col * (ICON_SIZE + GAP));
            int iconY = START_Y + (row * (ICON_SIZE + 35));

            // Check collision
            if (x >= iconX && x <= iconX + ICON_SIZE &&
                y >= iconY && y <= iconY + ICON_SIZE) {
                
                highlightApp(i);

                if (i == apps.size()) {
                    Serial.println("Launch File Browser");
                } else {
                    AppShortcut &app = apps[i];
                    Serial.print("Launching: "); Serial.println(app.name);
                    
                    if (app.type == APP_INTERNAL) {
                        if (app.execPath == "SYS_SETTINGS") {
                            system->launchApp((u8_t)1);
                        }
                        else if (app.execPath == "SYS_CHAT") {
                            system->launchApp((u8_t)2);
                        }
                    }
                }
                return;
            }
        }
    }
};