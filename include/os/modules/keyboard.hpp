#pragma once
#include "themes/theme_structure.hpp"

const char KEY_LAYOUT_LOWER[] = "qwertyuiopasdfghjklzxcvbnm";
const char KEY_LAYOUT_UPPER[] = "QWERTYUIOPASDFGHJKLZXCVBNM";
const char KEY_LAYOUT_NUM[]   = "1234567890-/:;()$&@\".,?!'#"; 

class VirtualKeyboard {
private:
    HardwareManager* hw;
    ThemePalette* theme;
    
    String buffer = "";
    String prompt = "";
    bool isFinished = false;
    bool isCancelled = false;
    bool shiftActive = false;
    bool numActive = false;
    bool needsRedraw = true;

    // --- PORTRAIT DIMENSIONS ---
    const int KEY_W = 21;  
    const int KEY_H = 38;  
    const int GAP = 3;     // Slightly more gap for the shadow
    const int START_X = 4; 
    const int START_Y = 150;

    const int KEYS_PER_ROW[3] = {10, 9, 7}; 
    const int ROW_OFFSET_X[3] = {0, 11, 35}; 

public:
    void init(HardwareManager* h, ThemePalette* t) { hw = h; theme = t; }

    void begin(String title, String initialValue = "") {
        prompt = title;
        buffer = initialValue;
        isFinished = false; isCancelled = false;
        shiftActive = false; numActive = false;
        needsRedraw = true;
    }

    void update() {
        if (needsRedraw) { draw(); needsRedraw = false; }
        handleTouch();
    }

    bool isDone() { return isFinished; }
    bool wasCancelled() { return isCancelled; }
    String getResult() { return buffer; }

private:
    // --- 3D BUTTON RENDERER ---
    void drawButton(int x, int y, int w, int h, const char* label, uint16_t bgCol, uint16_t txtCol, bool isSpecial = false) {
        int r = 5; // Radius
        int shadowOffset = 3;
        
        // 1. Draw Shadow (Offset down-right)
        hw->tft.fillRoundRect(x, y + shadowOffset, w, h, r, theme->PANEL_SHADOW);

        // 2. Draw Main Button Body (Shifted up slightly relative to shadow)
        hw->tft.fillRoundRect(x, y, w, h, r, bgCol);
        
        // 3. Optional: Subtle Highlight on top edge
        // hw->tft.drawFastHLine(x+2, y, w-4, 0xFFFF); // Adds a "glossy" look if desired

        // 4. Text
        hw->tft.setTextColor(txtCol, bgCol);
        hw->tft.setTextDatum(textdatum_t::middle_center);
        // Use a different font for special keys if possible, otherwise default
        hw->tft.drawString(label, x + w/2, y + (h/2) + 1);
        hw->tft.setTextDatum(textdatum_t::top_left);
    }

    void draw() {
        // Clear Keyboard Area
        //hw->tft.fillRect(0, 90, hw->tft.width(), hw->tft.height()-90, theme->BG_COLOR);
        hw->tft.fillScreen(theme->BG_COLOR);
        // --- ELEGANT INPUT BOX ---
        // A clean line with text above it, rather than a box
        int boxY = 65;
        
        // Label
        hw->tft.setTextColor(theme->ACCENT_PRIMARY, theme->BG_COLOR);
        hw->tft.setTextDatum(textdatum_t::bottom_left);
        hw->tft.drawString(prompt, 10, boxY - 5);
        
        // Input Value
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->BG_COLOR);
        hw->tft.setTextDatum(textdatum_t::middle_left);
        String displayBuffer = buffer;
        if(displayBuffer.length() > 18) displayBuffer = "..." + displayBuffer.substring(displayBuffer.length()-18);
        hw->tft.drawString(displayBuffer + "_", 10, boxY + 15);
        
        // Underline (The "Textbox")
        hw->tft.drawLine(5, boxY + 30, 235, boxY + 30, theme->BORDER_COLOR);
        hw->tft.drawLine(5, boxY + 31, 235, boxY + 31, theme->PANEL_SHADOW); // Shadow for line

        // --- KEYS ---
        const char* currentLayout = numActive ? KEY_LAYOUT_NUM : (shiftActive ? KEY_LAYOUT_UPPER : KEY_LAYOUT_LOWER);
        int charIndex = 0;

        for (int row = 0; row < 3; row++) {
            int numKeys = KEYS_PER_ROW[row];
            int xOffset = ROW_OFFSET_X[row];
            for (int col = 0; col < numKeys; col++) {
                int x = START_X + xOffset + (col * (KEY_W + GAP));
                int y = START_Y + (row * (KEY_H + GAP));
                char keyStr[2] = {currentLayout[charIndex], 0};
                
                // Color Logic: Normal keys vs Action keys
                drawButton(x, y, KEY_W, KEY_H, keyStr, theme->PANEL_BG, theme->TEXT_MAIN);
                charIndex++;
            }
        }

        // --- FUNCTION ROW ---
        int yFn = START_Y + (3 * (KEY_H + GAP));
        int xPos = START_X;

        // Mode Key
        drawButton(xPos, yFn, 30, KEY_H, numActive ? "Ab" : "12", theme->PANEL_BG, theme->TEXT_MUTED);
        xPos += 30 + GAP;

        // Shift Key (Active State changes color)
        uint16_t shiftBg = shiftActive ? theme->ACCENT_PRIMARY : theme->PANEL_BG;
        uint16_t shiftTxt = shiftActive ? theme->TEXT_MAIN : theme->TEXT_MUTED;
        drawButton(xPos, yFn, 30, KEY_H, "^", shiftBg, shiftTxt, true);
        xPos += 30 + GAP;

        // Space
        drawButton(xPos, yFn, 85, KEY_H, "", theme->PANEL_BG, theme->TEXT_MAIN); // Empty label for clean look
        // Little icon on spacebar
        hw->tft.drawFastHLine(xPos-85-GAP + 30, yFn + KEY_H/2, 25, theme->TEXT_MUTED);
        xPos += 85 + GAP;
        
        // Backspace (Red accent)
        drawButton(xPos, yFn, 30, KEY_H, "<", theme->ACCENT_ALERT, theme->TEXT_MAIN, true);
        xPos += 30 + GAP;
        
        // OK (Green/Blue accent)
        drawButton(xPos, yFn, 45, KEY_H, "OK", theme->ACCENT_PRIMARY, theme->TEXT_MAIN, true);
    }

    void handleTouch() {
        if (!hw->isTouching) return;
        delay(150);
        
        int tx = hw->touchX;
        int ty = hw->touchY;
        
        // Simple hit detection (Simplified for brevity, same logic as before)
        int rowIdx = (ty - START_Y) / (KEY_H + GAP);

        if (rowIdx >= 0 && rowIdx < 3) {
            int xOffset = ROW_OFFSET_X[rowIdx];
            if (tx < START_X + xOffset) return;
            int colIdx = (tx - START_X - xOffset) / (KEY_W + GAP);
            if (colIdx >= 0 && colIdx < KEYS_PER_ROW[rowIdx]) {
                int charIndex = 0;
                for(int r=0; r<rowIdx; r++) charIndex += KEYS_PER_ROW[r];
                charIndex += colIdx;
                const char* currentLayout = numActive ? KEY_LAYOUT_NUM : (shiftActive ? KEY_LAYOUT_UPPER : KEY_LAYOUT_LOWER);
                if (charIndex < 26) { 
                    if (buffer.length() < 30) buffer += currentLayout[charIndex];
                    if (shiftActive) { shiftActive = false; }
                    needsRedraw = true;
                }
            }
        } else if (rowIdx == 3) {
             // Re-map x coords based on widths in draw()
            if (tx >= 4 && tx < 34) { numActive = !numActive; shiftActive = false; needsRedraw = true; }
            else if (tx >= 36 && tx < 66) { if (!numActive) { shiftActive = !shiftActive; needsRedraw = true; } }
            else if (tx >= 68 && tx < 153) { if (buffer.length() < 30) buffer += " "; needsRedraw = true; }
            else if (tx >= 155 && tx < 185) { if (buffer.length() > 0) buffer.remove(buffer.length() - 1); needsRedraw = true; }
            else if (tx >= 187) { isFinished = true; }
        }
    }
};