#pragma once
#include <vector>
#include <ArduinoJson.h>

#include "hal/hal.hpp"
#include "../interfaces/application_link_interface.hpp"

class AppRegistry {
private:
    std::vector<AppShortcut> apps;
    HardwareManager* hw;
    const char* REGISTRY_FILE = "/apps.json";

public:
    void init(HardwareManager* h) {
        hw = h;
        loadRegistry();
    }

    void loadRegistry() {
        apps.clear();
        
        // 1. Always add Hardcoded System Apps first
        apps.push_back({"Settings", "", 0x738E, APP_INTERNAL, "SYS_SETTINGS"});
        apps.push_back({"Chat", "", 0x3333, APP_INTERNAL, "SYS_CHAT"});

        // 2. Load External Apps from SD
        if (hw->sdAvailable && SD.exists(REGISTRY_FILE)) {
            File file = SD.open(REGISTRY_FILE, "r");
            if (file) {
                DynamicJsonDocument doc(2048);
                DeserializationError error = deserializeJson(doc, file);
                if (!error) {
                    JsonArray array = doc.as<JsonArray>();
                    for (JsonObject obj : array) {
                        AppShortcut a;
                        a.name = obj["name"].as<String>();
                        a.color = obj["color"];
                        a.type = APP_EXTERNAL;
                        a.execPath = obj["path"].as<String>();
                        apps.push_back(a);
                    }
                }
                file.close();
            }
        }
    }

    void installApp(String name, String path, uint16_t color) {
        // Add to list
        AppShortcut newApp = {name, "", color, APP_EXTERNAL, path};
        apps.push_back(newApp);
        saveRegistry();
    }

    void saveRegistry() {
        if (!hw->sdAvailable) return;

        DynamicJsonDocument doc(2048);
        JsonArray array = doc.to<JsonArray>();

        // Only save EXTERNAL apps to JSON
        for (const auto& app : apps) {
            if (app.type == APP_EXTERNAL) {
                JsonObject obj = array.createNestedObject();
                obj["name"] = app.name;
                obj["path"] = app.execPath;
                obj["color"] = app.color;
            }
        }

        File file = SD.open(REGISTRY_FILE, "w");
        serializeJson(doc, file);
        file.close();
    }

    std::vector<AppShortcut>& getApps() { return apps; }
};