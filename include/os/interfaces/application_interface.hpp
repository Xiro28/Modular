#pragma once
#include "hal/hal.hpp"
#include "../../themes/theme_structure.hpp"

// Forward declaration
class Kernel; 

// APP ID structure:
// ID < 64 -> System/Internal Apps
// ID >=64 -> External/User Apps

class Application {
protected:
    HardwareManager* hw;
    Kernel* system;
    ThemePalette* theme;

    u8_t pid;
    u8_t appID;
    bool needsRedraw = true;

    Application() : pid(0), appID(255) {} // Default constructor
    Application(u8_t id) : pid(0), appID(id) {} // Constructor with App ID

public:
    // Setup references when App starts
    void inject(HardwareManager* h, Kernel* s, ThemePalette* t) {
        hw = h; system = s; theme = t;
    }

    void forceRedraw() {
        needsRedraw = true;
    }
    void setPID(u8_t id) { pid = id; }
    u8_t getPID() const { return pid; }
    u8_t getAppID() const { return appID; }


    virtual void onStart() = 0;   // Setup
    virtual void onUpdate() = 0;  // Loop
    virtual void onDraw() = 0;    // Draw (called when needed)
    virtual void onExit() = 0;    // Cleanup before switch
    virtual ~Application() {}
};