#pragma once
#include "../../hal/hal.hpp" // Il tuo HardwareManager
#include "../../themes/theme_structure.hpp"   // Il tuo ThemePalette

enum ToastType {
    TOAST_INFO,
    TOAST_SUCCESS,
    TOAST_ERROR,
    TOAST_WARNING
};

class ToastManager {
private:    
    HardwareManager* hw;
    ThemePalette* theme;

    // Stato del Toast
    String message;
    uint16_t msgColor;
    uint16_t bgColor;
    
    bool isVisible = false;
    unsigned long startTime = 0;
    unsigned long duration = 2000;
    
    // Animazione
    float currentY;  // Posizione Y attuale
    float targetY;   // Posizione Y target (visibile)
    float hiddenY;   // Posizione Y nascosta (fuori schermo)
    bool lastState = false;
    bool changed = false;

    // Costruttore privato (Singleton)
    ToastManager() {}

public:
    // Accesso Singleton
    static ToastManager* getInstance() {
        static ToastManager* instance = nullptr;
        if (instance == nullptr) {
            instance = new ToastManager();
        }
        return instance;
    }

    // Inizializzazione (da chiamare nel Kernel::boot)
    void init(HardwareManager* h, ThemePalette* t) {
        hw = h;
        theme = t;
        hiddenY = hw->tft.height() + 10; // Appena sotto lo schermo
        targetY = hw->tft.height() - 60; // Posizione visibile (dal basso)
        currentY = hiddenY;
    }

    // Mostra un messaggio
    void show(String msg, ToastType type = TOAST_INFO, int ms = 2500) {
        message = msg;
        duration = ms;
        startTime = millis();
        isVisible = true;
        changed = true;


        // Imposta colori in base al tipo
        switch (type) {
            case TOAST_SUCCESS: msgColor = theme->TEXT_MAIN; bgColor = 0x07E0; /* Verde scuro */ break; // O usa theme->ACCENT_PRIMARY
            case TOAST_ERROR:   msgColor = theme->TEXT_MAIN; bgColor = theme->ACCENT_ALERT; break;
            case TOAST_WARNING: msgColor = theme->BG_COLOR;  bgColor = theme->ACCENT_WARN; break;
            case TOAST_INFO:    
            default:            msgColor = theme->TEXT_MAIN; bgColor = theme->PANEL_SHADOW; break;
        }
        
        // Se vuoi sovrascrivere con i colori del tema corrente per coerenza:
        if (type == TOAST_SUCCESS) bgColor = theme->ACCENT_PRIMARY;
    }

    // Da chiamare nel loop principale (disegna sopra tutto)
    void update() {
        if (!hw) return;

        unsigned long now = millis();

        // 1. Logica di Timeout
        if (isVisible && (now - startTime > duration)) {
            isVisible = false; // Inizia a nascondersi
        }

        // 2. Calcolo Animazione (Slide semplice)
        float goal = isVisible ? targetY : hiddenY;
        
        // Interpolazione lineare (Lerp) per fluidità
        // Se la differenza è piccola, scatta alla posizione per evitare micro-movimenti
        /*if (abs(goal - currentY) < 1.0) {
            currentY = goal;
        } else {
            currentY += (goal - currentY) * 0.2; // 0.2 è la velocità dello slide
        }*/

        currentY = goal;

        // 3. Disegno (Solo se è visibile parzialmente o totalmente)
        //if (currentY < hiddenY - 1) 
        if (lastState != isVisible || changed)
        {
            draw();
            changed = false;
        }
        lastState = isVisible;
    }

    bool isActive() const {
        return isVisible || (currentY < hiddenY - 1);
    }

private:
    void draw() {
        // Calcola larghezza dinamica in base al testo
        int textW = hw->tft.textWidth(message);
        int padding = 30;
        int toastW = textW + padding;
        int toastH = 40;
        
        // Centra orizzontalmente
        int screenW = hw->tft.width();
        int x = (screenW - toastW) / 2;
        int y = (int)currentY;

        // 1. Ombra (Shadow)
        hw->tft.fillRoundRect(x + 2, y + 2, toastW, toastH, 20, 0x0000); // Nero puro o PANEL_SHADOW

        // 2. Sfondo Toast
        hw->tft.fillRoundRect(x, y, toastW, toastH, 20, bgColor);

        // 3. Bordo sottile (Opzionale, per eleganza su sfondo scuro)
        hw->tft.drawRoundRect(x, y, toastW, toastH, 20, theme->TEXT_MUTED);

        // 4. Testo
        hw->tft.setTextColor(msgColor, bgColor);
        hw->tft.setTextDatum(textdatum_t::middle_center);
        hw->tft.drawString(message, x + toastW/2, y + toastH/2);
        hw->tft.setTextDatum(textdatum_t::top_left); // Reset
    }
};