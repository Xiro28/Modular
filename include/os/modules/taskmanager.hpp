#pragma once
#include <vector>
#include <algorithm>
#include "../interfaces/application_interface.hpp"

class TaskManager {
private:
    std::vector<Application*> registeredApps;
public:
    void registerApplication(Application* app) {
        if (app) registeredApps.push_back(app);
    }
    Application* openRegisteredApplication(uint8_t id, void* hw, void* sys, void* theme) {
        for (auto* app : registeredApps) {
            if (app && app->getAppID() == id) {
                app->inject((HardwareManager*)hw, (Kernel*)sys, (ThemePalette*)theme);
                return app;
            }
        }
        return nullptr;
    }
};