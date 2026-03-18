#pragma once
#include "interfaces/application_interface.hpp"
#include "modules/application_manager.hpp"
#include "modules/taskmanager.hpp"
#include "modules/keyboard.hpp"
#include "themes/theme_structure.hpp"
#include "daas/daas_interfaces.hpp"
#include <string>

class Kernel {
private:
    TaskManager taskManager;
    HardwareManager hardware;
    ThemePalette *currentTheme = nullptr;
    VirtualKeyboard keyboard;
    Application* currentApp = nullptr;
    Vector<din_t> discoveredNodes;
    
public:
    AppRegistry registry;
    
    bool wifiConnected = false;
    bool btEnabled = false;
    bool daasConnected = false;
    bool daasWifiDriver = false;
    bool daasBtDriver = false;

    void boot();
    void bootAnimation();
    inline void registerApplication(Application* app) { taskManager.registerApplication(app); }
    void launchApp(uint8_t appID);
    void run();

    void addNode(din_t din) {
        for (uint32_t i = 0; i < discoveredNodes.size(); i++) {
            if (discoveredNodes[i] == din || discoveredNodes[i]>>44 == din>>44) return;
        }
        discoveredNodes.push_back(din);
    }

    Vector<din_t>& getDiscoveredNodes() { return discoveredNodes; }
    ThemePalette*& getTheme() { return currentTheme; }
    HardwareManager* getHW() { return &hardware; }
    VirtualKeyboard* getKeyboard() { return &keyboard; }
};