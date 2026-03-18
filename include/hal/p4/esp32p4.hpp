#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include "lvgl.h"

// Il BSP gestisce nativamente tutta l'inizializzazione hardware e LVGL
#include "bsp/esp-bsp.h"

// Stub per compatibilità con il codice esistente
namespace textdatum_t {
    enum datum { top_left, top_center, top_right, middle_left, middle_center, middle_right, bottom_left, bottom_center, bottom_right };
}
namespace fonts {
    const int efontCN_14 = 1;
    const int efontCN_24 = 2;
    const int Font4 = 4;
}

class HardwareManager {
public:
    // Rimosso LovyanGFX_Adapter e MockWiFi globale per evitare crash all'avvio

    void init() {
        printf("SYSTEM: Inizializzazione Hardware P4 (MIPI-DSI)...\n");
        
        // 1. Inizializza il Display tramite BSP
        // bsp_display_start() restituisce lv_disp_t*, non esp_err_t
        lv_disp_t* disp = bsp_display_start();
        
        if (disp == nullptr) {
            printf("ERRORE: Inizializzazione Display fallita!\n");
            return;
        }

        // 2. Accendi la retroilluminazione
        bsp_display_backlight_on();
        
        printf("SYSTEM: Display e Touch pronti.\n");
    }

    // Stub per mantenere la compatibilità della struttura del Kernel
    void updateInput() {
        // Gestito internamente dai task del BSP e di LVGL
    }

    bool isTouchInRect(int x, int y, int w, int h) {
        // LVGL gestisce i click autonomamente tramite i callback degli oggetti
        return false; 
    }
    
    bool saveCurrentWifi() { return true; }
};