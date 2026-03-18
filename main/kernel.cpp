#include "os/kernel.hpp"
#include "themes/default_theme.hpp"
#include "os/modules/toastmessages.hpp"
#include "bsp/esp-bsp.h"

ThemePalette DEFAULT_THEME = { 0x1082, 0x2124, 0x0841, 0xFFFF, 0x9492, 0x04F9, 0xE46C, 0xF800, 0x4A69, 0x1082 };

void Kernel::boot() {
    currentTheme = &DEFAULT_THEME;
    hardware.init();
    registry.init(&hardware);
    discoveredNodes.clear();
    discoveredNodes.reserve(10);
    keyboard.init(&hardware, currentTheme);
    ToastManager::getInstance()->init(&hardware, currentTheme);
}

void Kernel::run() {
    if (currentApp) {
        bsp_display_lock(0);
        currentApp->onUpdate();
        ToastManager::getInstance()->update();
        bsp_display_unlock();
    }
}

void Kernel::bootAnimation() {}

void Kernel::launchApp(uint8_t appID) {
    Application* oldApp = currentApp;
    
    currentApp = taskManager.openRegisteredApplication(appID, &hardware, this, currentTheme);
    
    if (currentApp) {
        currentApp->onStart();
    }
    
    if (oldApp) {
        oldApp->onExit();
    }
}