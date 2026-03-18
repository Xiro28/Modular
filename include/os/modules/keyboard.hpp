#pragma once
#include <string>

// STUBBED: In pure LVGL, apps generate their own lv_keyboard widgets directly.
// This stub prevents kernel.cpp from throwing compilation errors.
class VirtualKeyboard {
public:
    void init(void* hw, void* theme) {}
    void begin(std::string promptText) {}
    void update() {}
    void draw() {}
    void handleTouch() {}
    bool isDone() { return false; }
    bool wasCancelled() { return false; }
    std::string getResult() { return ""; }
};