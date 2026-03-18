#pragma once
#include "os/interfaces/application_interface.hpp"
#include "lvgl.h"

class BootApp : public Application {
private:
    lv_obj_t* screen = nullptr;
    lv_obj_t* label = nullptr;
    lv_obj_t* spinner = nullptr;

    static void anim_ready_cb(lv_anim_t* a) {
        BootApp* app = (BootApp*)a->user_data;
        // Transizione morbida verso la Home
        lv_timer_create([](lv_timer_t* t){
            ((BootApp*)t->user_data)->system->launchApp(0);
            lv_timer_del(t);
        }, 1200, app);
    }
public:
    BootApp() : Application(99) {}
    void onStart() override {
        screen = lv_obj_create(NULL);
        // Sfondo con gradiente radiale scuro
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x05020A), 0);
        lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x150F24), 0);
        lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

        // Testo del logo
        label = lv_label_create(screen);
        lv_label_set_text(label, "Modular OS");
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0); // Font più grande se disponibile, altrimenti 14
        lv_obj_align(label, LV_ALIGN_CENTER, 0, -20);
        lv_obj_set_style_opa(label, 0, 0);

        // Spinner di caricamento elegante
        spinner = lv_spinner_create(screen, 1000, 60);
        lv_obj_set_size(spinner, 40, 40);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 30);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(0x2B1E4A), LV_PART_MAIN);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(0xA855F7), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner, 4, LV_PART_MAIN);
        lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);
        lv_obj_set_style_opa(spinner, 0, 0);

        // Animazione Fade-in
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, label);
        lv_anim_set_values(&a, 0, 255);
        lv_anim_set_time(&a, 1000);
        lv_anim_set_exec_cb(&a, [](void* var, int32_t v){ lv_obj_set_style_opa((lv_obj_t*)var, v, 0); });
        
        lv_anim_t b = a; // Copia l'animazione per lo spinner
        lv_anim_set_var(&b, spinner);
        lv_anim_set_time(&b, 1500);
        b.user_data = this;
        lv_anim_set_ready_cb(&b, anim_ready_cb);

        lv_anim_start(&a);
        lv_anim_start(&b);
        lv_scr_load(screen);
    }
    void onExit() override { if (screen) lv_obj_del(screen); screen = nullptr; }
    void onUpdate() override {}
    void onDraw() override {}
};