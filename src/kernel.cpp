#include "os/kernel.hpp"
#include "themes/default_theme.hpp"
#include "os/modules/toastmessages.hpp"

ThemePalette DEFAULT_THEME = {
    0x1082, // Deep Dark Slate (Background)
    0x2124, // Lighter Slate (Key/Tile surface)
    0x0841, // Dark Shadow (For depth)
    0xFFFF, // White
    0x9492, // Gray
    0x04F9, // Neon Blue (Active)
    0xE46C, // Soft Orange
    0xF800, // Red
    0x4A69, // Subtle border
    0x1082  // Same as BG for "Clean" look
};

void Kernel::boot() {
    currentTheme = &DEFAULT_THEME; // Later: Load from JSON

    hardware.init(); // Init SD first
    registry.init(&hardware); // Then load apps
    
    bootAnimation();
    
    discoveredNodes.clear();
    discoveredNodes.reserve(10);
    
    node.doInit(0x0, 0x0); // Dummy DIN/SID for now
    
    node.setAcceptRequestsLevel(1);
    node.setDDOPolicy(ddo_policy_skip_on_failure);
    node.setDiscoveryState(discovery_sender_only);
    node.setATSMaxError(250);
    
    keyboard.init(&hardware, currentTheme);
    ToastManager::getInstance()->init(&hardware, currentTheme);

    delay(500);
}

void Kernel::run() 
 {
    static bool wasToastActive = false;
    bool isToastActive = ToastManager::getInstance()->isActive();
    
    node.doPerform(PERFORM_CORE_NO_THREAD);

    hardware.updateInput();

    if (currentApp) {
        currentApp->onUpdate();
    }

    ToastManager::getInstance()->update();

    if (wasToastActive && !isToastActive) {
        if (currentApp) {
            currentApp->forceRedraw(); 
        }
    }
    
    wasToastActive = isToastActive;
}

// Funzione di Easing Elastico per l'effetto "Snap" magnetico
float easeOutElastic(float x) {
    const float c4 = (2 * PI) / 3;
    return x == 0 ? 0 : x == 1 ? 1 : pow(2, -10 * x) * sin((x * 10 - 0.75) * c4) + 1;
}

void Kernel::bootAnimation() {
    int w = hardware.tft.width();
    int h = hardware.tft.height();

    // --- CONFIGURAZIONE GEOMETRICA (Una croce modulare) ---
    int bS = 45;   // Dimensione lato blocco (quadrato)
    int gap = 4;   // Spazio piccolissimo tra i moduli
    int r = 8;     // Arrotondamento moderno

    // Coordinate del blocco centrale (Core)
    int cx = (w - bS) / 2;
    int cy = (h - bS) / 2 - 20; // Leggermente su

    // Target Positions per i 4 moduli periferici (Top, Bot, Left, Right)
    struct Rect { int x; int y; };
    Rect t[4];
    t[0] = {cx, cy - bS - gap}; // Top
    t[1] = {cx, cy + bS + gap}; // Bottom
    t[2] = {cx - bS - gap, cy}; // Left
    t[3] = {cx + bS + gap, cy}; // Right

    // Start Positions (Fuori schermo)
    Rect s[4];
    int off = 120; // Distanza di partenza
    s[0] = {t[0].x, t[0].y - off};
    s[1] = {t[1].x, t[1].y + off};
    s[2] = {t[2].x - off, t[2].y};
    s[3] = {t[3].x + off, t[3].y};

    // Colori
    uint16_t bg = currentTheme->BG_COLOR;
    uint16_t outlineCol = currentTheme->PANEL_SHADOW; // Colore per il "blueprint"
    uint16_t activeCol = currentTheme->ACCENT_PRIMARY; // Colore per i moduli attivi

    hardware.tft.fillScreen(bg);
    delay(100);

    // --- FASE 1: IL BLUEPRINT (La possibilità) ---
    // Disegna i contorni vuoti dove andranno i blocchi.
    // Questo suggerisce che lo spazio è pronto per essere riempito dall'utente.
    for(int i=0; i<4; i++) hardware.tft.drawRoundRect(t[i].x, t[i].y, bS, bS, r, outlineCol);
    hardware.tft.drawRoundRect(cx, cy, bS, bS, r, outlineCol); // Core outline
    delay(300);


    // --- FASE 2: IL CORE (La base si materializza) ---
    // Un semplice fade-in o scale-up del blocco centrale
    for(int i=0; i<=bS; i+=5) {
        int currentSize = i;
        int x = cx + (bS - currentSize)/2;
        int y = cy + (bS - currentSize)/2;
        // Pulisci e disegna che cresce
        hardware.tft.fillRoundRect(cx, cy, bS, bS, r, bg); // Pulisci area max
        hardware.tft.fillRoundRect(x, y, currentSize, currentSize, r, activeCol);
        delay(15);
    }
    delay(200);

    // --- FASE 3: L'ASSEMBLAGGIO (I moduli arrivano) ---
    // Usiamo l'elastic easing per un effetto di aggancio magnetico soddisfacente
    int steps = 90;
    Rect prev[4];
    for(int i=0; i<4; i++) prev[i] = s[i];

    for (int step = 0; step <= steps; step++) {
        float t_lin = (float)step / steps;
        float progress = easeOutElastic(t_lin); // Effetto rimbalzo/magnete

        for (int i = 0; i < 4; i++) {
            int curX = s[i].x + (t[i].x - s[i].x) * progress;
            int curY = s[i].y + (t[i].y - s[i].y) * progress;

            // Cancella scia (Wipe pulito)
            if (curX != prev[i].x || curY != prev[i].y) {
                hardware.tft.fillRect(prev[i].x-1, prev[i].y-1, bS+2, bS+2, bg);
            }
            
            // Ridisegna il blueprint sotto se il blocco si è spostato (opzionale, per pulizia estrema)
            // hardware.tft.drawRoundRect(t[i].x, t[i].y, bS, bS, r, outlineCol);

            // Disegna il modulo in arrivo
            hardware.tft.fillRoundRect(curX, curY, bS, bS, r, activeCol);
            
            prev[i].x = curX; prev[i].y = curY;
        }
        delay(5);
    }

    // --- FASE 4: ATTIVAZIONE (Il sistema prende vita) ---
    // Un lampo di luce (bianco -> accento) su tutta la struttura assemblata
    delay(100);
    uint16_t flashCol = currentTheme->TEXT_MAIN; // Bianco brillante
    
    // Flash ON
    hardware.tft.fillRoundRect(cx, cy, bS, bS, r, flashCol); // Core
    for(int i=0; i<4; i++) hardware.tft.fillRoundRect(t[i].x, t[i].y, bS, bS, r, flashCol);
    delay(60);
    
    // Flash OFF (Ritorna al colore accento)
    hardware.tft.fillRoundRect(cx, cy, bS, bS, r, activeCol); // Core
    for(int i=0; i<4; i++) hardware.tft.fillRoundRect(t[i].x, t[i].y, bS, bS, r, activeCol);
    
    delay(300);

    // --- TESTO (Pulito e gerarchico) ---
    int textY = t[1].y + bS + 35; // Spazio sotto il modulo inferiore
    hardware.tft.setTextDatum(textdatum_t::top_center);

    // Titolo principale
    hardware.tft.setTextColor(currentTheme->TEXT_MAIN, bg);
    hardware.tft.setFont(&fonts::efontCN_24); // Font più grande se possibile
    hardware.tft.drawString("MODULAR", w/2, textY);

    // Sottotitolo (che esprime il concetto)
    hardware.tft.setFont(&fonts::efontCN_14); // Font più piccolo
    hardware.tft.setTextColor(currentTheme->TEXT_MUTED, bg);
    // Dissolvenza simulata per il sottotitolo
    const char* sub = "Powered By DaaS";
    
    int subY = textY + 35;
    hardware.tft.setTextColor(currentTheme->PANEL_SHADOW, bg); // Molto scuro
    hardware.tft.drawString(sub, w/2, subY);
    delay(150);
    hardware.tft.setTextColor(currentTheme->BORDER_COLOR, bg); // Medio
    hardware.tft.drawString(sub, w/2, subY);
    delay(150);
    hardware.tft.setTextColor(currentTheme->TEXT_MUTED, bg); // Finale
    hardware.tft.drawString(sub, w/2, subY);

    delay(2000); // Pausa finale per ammirare il risultato
}
void Kernel::launchApp(u8_t appID) {
    const auto sys_app = taskManager.openRegisteredApplication(appID);

    if (sys_app != nullptr) {
        sys_app->inject(&hardware, this, currentTheme);
        hardware.resetScreen(currentTheme->BG_COLOR);
        sys_app->onDraw();

        currentApp = sys_app;
    }
}