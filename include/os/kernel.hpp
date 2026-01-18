#pragma once
#include "interfaces/application_interface.hpp"
#include "modules/application_manager.hpp"
#include "modules/taskmanager.hpp"
#include "modules/keyboard.hpp"
#include "themes/theme_structure.hpp"

#include "daas/daas_interfaces.hpp"


class Kernel {
private:
    IDaasApiEvent* eventHandler = new daas_node_event(this);
    DaasAPI node = DaasAPI(eventHandler);
    TaskManager taskManager;
    HardwareManager hardware;
    ThemePalette *currentTheme = nullptr;
    VirtualKeyboard keyboard;
    Application* currentApp = nullptr;

    Vector<din_t> discoveredNodes;
    
    public:
    
    AppRegistry registry;

    void boot();
    void bootAnimation();

    inline void registerApplication(Application* app) { taskManager.registerApplication(app); }

    void launchApp(u8_t appID);

    void run();

    void addNode(din_t din) {
        // Check if already present
        for (u32_t i = 0; i < discoveredNodes.size(); i++) {
            if (discoveredNodes[i] == din || discoveredNodes[i]>>44 == din>>44) return; // Already present
        }
        discoveredNodes.push_back(din);
    }

    Vector<din_t>& getDiscoveredNodes() {
        return discoveredNodes;
    }
    

    // API Accessors
    ThemePalette*& getTheme() { return currentTheme; }
    HardwareManager* getHW() { return &hardware; }
    DaasAPI* getNode() { return &node; }
    VirtualKeyboard* getKeyboard() { return &keyboard; }
    bool daasNetworkConnected = false;

    bool ddoPulled = false;
    String ddoPayload = "";
};