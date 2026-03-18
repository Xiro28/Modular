#pragma once
#include <string>
#include "lvgl.h"

enum ToastType { TOAST_INFO, TOAST_WARNING, TOAST_ERROR };

class ToastManager {
private:
    lv_obj_t* toast_label = nullptr;
    lv_timer_t* toast_timer = nullptr;

    ToastManager() {}

    static void timer_cb(lv_timer_t* timer) {
        ToastManager* tm = (ToastManager*)timer->user_data;
        if (tm->toast_label) {
            lv_obj_del(tm->toast_label);
            tm->toast_label = nullptr;
        }
        lv_timer_del(timer);
        tm->toast_timer = nullptr;
    }

public:
    static ToastManager* getInstance() {
        static ToastManager instance; // Clean Singleton pattern
        return &instance;
    }

    // Stubs to keep kernel.cpp happy
    void init(void* hw, void* theme) {}
    void update() {}
    void draw() {}

    void show(std::string msg, ToastType type = TOAST_INFO, int ms = 2500) {
        if (toast_label) lv_obj_del(toast_label);
        if (toast_timer) lv_timer_del(toast_timer);

        // Create a floating label on the top-most system layer
        toast_label = lv_label_create(lv_layer_sys()); 
        lv_label_set_text(toast_label, msg.c_str());
        
        lv_obj_set_style_bg_opa(toast_label, LV_OPA_COVER, 0);
        
        uint32_t bgColor = 0x333333; // Default Dark
        if(type == TOAST_ERROR) bgColor = 0xFF0000;
        if(type == TOAST_WARNING) bgColor = 0xFFA500;
        
        lv_obj_set_style_bg_color(toast_label, lv_color_hex(bgColor), 0);
        lv_obj_set_style_text_color(toast_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_pad_all(toast_label, 15, 0);
        lv_obj_set_style_radius(toast_label, 10, 0);
        
        // Center at the bottom of the screen
        lv_obj_align(toast_label, LV_ALIGN_BOTTOM_MID, 0, -20);

        // Start destruction timer
        toast_timer = lv_timer_create(timer_cb, ms, this);
    }
};