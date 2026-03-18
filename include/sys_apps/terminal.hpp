#pragma once
#include "os/interfaces/application_interface.hpp"
#include <string>
#include "lvgl.h"

class TerminalApp : public Application {
private:
    lv_obj_t* screen = nullptr;
    lv_obj_t* term_ta = nullptr;
    uint32_t last_tick = 0;
    int step = 0;

    // Finti log di sistema per dare l'effetto "Terminale"
    const char* log_lines[7] = {
        "> Root access granted.\n",
        "> Initializing DaaS drivers...\n",
        "> Memory allocated: 4802KB [OK]\n",
        "> Mounting internal SD...\n",
        "> Network handshake established.\n",
        "> Awaiting secure connection...\n",
        "> SYSTEM READY.\n_"
    };

public:
    TerminalApp() : Application(100) {}

    void onStart() override {
        screen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0); // Nero puro
        lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);

        // Layout Flex principale per evitare sovrapposizioni
        lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(screen, 0, 0);

        // --- HEADER ---
        lv_obj_t* head = lv_obj_create(screen);
        lv_obj_set_size(head, LV_PCT(100), 60);
        lv_obj_set_style_bg_color(head, lv_color_hex(0x111111), 0);
        lv_obj_set_style_border_width(head, 0, 0);
        lv_obj_set_style_pad_all(head, 0, 0);
        lv_obj_set_scrollbar_mode(head, LV_SCROLLBAR_MODE_OFF);
        
        lv_obj_t* back = lv_btn_create(head);
        lv_obj_set_size(back, 45, 45);
        lv_obj_align(back, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_set_style_bg_opa(back, 0, 0); // Bottone invisibile, si vede solo l'icona
        lv_obj_set_style_shadow_width(back, 0, 0);
        lv_obj_add_event_cb(back, [](lv_event_t* e){
            ((TerminalApp*)lv_event_get_user_data(e))->system->launchApp(0); // Torna alla Home
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_t* back_lbl = lv_label_create(back);
        lv_label_set_text(back_lbl, LV_SYMBOL_LEFT);
        lv_obj_set_style_text_color(back_lbl, lv_color_hex(0x10B981), 0); // Verde hacker
        lv_obj_center(back_lbl);

        lv_obj_t* title = lv_label_create(head);
        lv_label_set_text(title, "root@modular:~");
        lv_obj_set_style_text_color(title, lv_color_hex(0x10B981), 0);
        lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

        // --- AREA TESTO TERMINALE ---
        term_ta = lv_label_create(screen);
        lv_obj_set_width(term_ta, LV_PCT(100));
        lv_obj_set_flex_grow(term_ta, 1); // Riempi tutto lo schermo rimanente!
        lv_obj_set_style_bg_color(term_ta, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_color(term_ta, lv_color_hex(0x10B981), 0); 
        lv_obj_set_style_text_font(term_ta, &lv_font_montserrat_14, 0);
        lv_obj_set_style_pad_all(term_ta, 15, 0);
        lv_label_set_text(term_ta, "Modular OS v1.3.0 Kernel\n> Booting...\n");
        
        step = 0;
        last_tick = lv_tick_get();
        lv_scr_load(screen);
    }

    void onUpdate() override {
        // Aggiunge una riga ogni 700ms per simulare il caricamento del terminale
        if (step < 7 && lv_tick_get() - last_tick > 700) {
            // Concateniamo il testo precedente con la nuova riga
            std::string current_text = lv_label_get_text(term_ta);
            current_text += log_lines[step];
            lv_label_set_text(term_ta, current_text.c_str());
            
            step++;
            last_tick = lv_tick_get();
        }
    }

    void onExit() override { if (screen) lv_obj_del(screen); screen = nullptr; }
    void onDraw() override {}
};