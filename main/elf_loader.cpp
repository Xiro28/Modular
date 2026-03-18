#include "hal/hal.hpp"
/*#include "sdk.h"

HardwareManager hw;
HardwareManager* instance = nullptr;

// --- 1. FUNZIONI WRAPPER (Il ponte tra App e Hardware) ---
// Queste devono essere statiche o globali per avere un puntatore pulito.

static void k_drawRect(int x, int y, int w, int h, uint16_t color) {
    hw.tft.fillRect(x, y, w, h, color);
}

static void k_drawText(const char* text, int x, int y) {
    hw.tft.drawString(text, x, y);
}

static void k_delay(uint32_t ms) {
    delay(ms);
}

static unsigned long k_millis() {
    return millis();
}

static int k_rand() {
    return rand();
}

// --- 2. IL CARICATORE DI APP ---
void loadAndRun(const char* path) {
    if (!hw.sdAvailable || !SD.exists(path)) {
        hw.tft.drawString("App not found!", 10, 10);
        return;
    }

    File f = SD.open(path);
    size_t size = f.size();

    // Allocazione Memoria ESEGUIBILE (IRAM)
    // CRUCIALE: Usare MALLOC_CAP_EXEC
    void* code_mem = heap_caps_malloc(size, MALLOC_CAP_EXEC);
    
    if (!code_mem) {
        hw.tft.drawString("RAM Error!", 10, 10);
        f.close();
        return;
    }

    // Copia il codice binario
    f.readBytes((char*)code_mem, size);
    f.close();

    // Prepara la valigetta API
    SystemAPI api;
    api.screenWidth  = hw.tft.width();
    api.screenHeight = hw.tft.height();
    api.drawRect     = &k_drawRect;
    api.drawText     = &k_drawText;
    api.delay        = &k_delay;
    api.millis       = &k_millis;
    api.rand         = &k_rand;
    // Funzioni C standard reali
    api.memcpy       = &memcpy;
    api.memset       = &memset;

    // Esegui l'App
    Serial.println("Running App...");
    app_entry_t app_main = (app_entry_t)code_mem;
    app_main(&api);
    
    // Pulizia
    heap_caps_free(code_mem);
    hw.tft.fillScreen(0);
    hw.tft.drawString("App Closed", 10, 10);
}
*/