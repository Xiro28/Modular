#pragma once
#include "os/interfaces/application_interface.hpp"
#include <string>
#include <vector>
#include "lvgl.h"

class HomeApp : public Application {
private:
    lv_obj_t* screen = nullptr;
    
    // Elementi dinamici
    lv_obj_t* time_lbl = nullptr;
    lv_obj_t* wifi_icon = nullptr;
    lv_obj_t* bt_icon = nullptr;
    lv_obj_t* daas_icon = nullptr;
    lv_obj_t* uptime_lbl = nullptr;

    uint32_t last_tick = 0;
    int mock_minutes = 30;
    int mock_hours = 10;
    uint32_t uptime_seconds = 0;
    
    int pending_app_id = -1;

    static void app_click_cb(lv_event_t* e) {
        HomeApp* app = (HomeApp*)lv_event_get_user_data(e);
        int id = (int)(uintptr_t)lv_event_get_target(e)->user_data;
        app->pending_app_id = id;

        // Effetto grafico "click" (abbassa l'opacità)
        lv_obj_t* icon_bg = lv_obj_get_child(lv_event_get_target(e), 0);
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, icon_bg);
        lv_anim_set_values(&a, 255, 100);
        lv_anim_set_time(&a, 100);
        lv_anim_set_playback_time(&a, 100);
        lv_anim_set_exec_cb(&a, [](void* var, int32_t v){ lv_obj_set_style_bg_opa((lv_obj_t*)var, v, 0); });
        lv_anim_start(&a);

        // Lancio sicuro dell'app
        lv_timer_create([](lv_timer_t* t){
            HomeApp* a = (HomeApp*)t->user_data;
            if(a->pending_app_id != -1) {
                a->system->launchApp(a->pending_app_id);
                a->pending_app_id = -1; 
            }
            lv_timer_del(t);
        }, 150, app);
    }

    // Creatore di icone responsive
    lv_obj_t* create_app_icon(lv_obj_t* parent, int id, const char* name, uint32_t color, const char* symbol, int badge_count, bool show_label) {
        lv_obj_t* btn = lv_btn_create(parent);
        // Dimensione base per il touch, adatta a schermi grandi
        lv_obj_set_size(btn, 100, show_label ? 120 : 80); 
        lv_obj_set_style_bg_opa(btn, 0, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 0, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
        
        btn->user_data = (void*)(uintptr_t)id;
        lv_obj_add_event_cb(btn, app_click_cb, LV_EVENT_CLICKED, this);

        // Il background dell'icona
        lv_obj_t* icn_bg = lv_obj_create(btn);
        lv_obj_set_size(icn_bg, 76, 76); 
        lv_obj_align(icn_bg, show_label ? LV_ALIGN_TOP_MID : LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_radius(icn_bg, 20, 0);
        lv_obj_clear_flag(icn_bg, LV_OBJ_FLAG_CLICKABLE); 
        lv_obj_set_scrollbar_mode(icn_bg, LV_SCROLLBAR_MODE_OFF);
        
        // Stile "Glass/Premium"
        lv_obj_set_style_bg_color(icn_bg, lv_color_hex(color), 0);
        lv_obj_set_style_bg_grad_color(icn_bg, lv_color_mix(lv_color_hex(0x000000), lv_color_hex(color), 140), 0);
        lv_obj_set_style_bg_grad_dir(icn_bg, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_border_width(icn_bg, 2, 0);
        lv_obj_set_style_border_color(icn_bg, lv_color_mix(lv_color_hex(0xFFFFFF), lv_color_hex(color), 100), 0);
        lv_obj_set_style_border_opa(icn_bg, LV_OPA_60, 0);
        
        // Simbolo al centro
        lv_obj_t* lbl = lv_label_create(icn_bg);
        lv_label_set_text(lbl, symbol);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(lbl);

        // Badge di notifica rosso (se > 0)
        if (badge_count > 0) {
            lv_obj_t* badge = lv_obj_create(btn);
            lv_obj_set_size(badge, 26, 26);
            lv_obj_align(badge, LV_ALIGN_TOP_RIGHT, -4, -4);
            lv_obj_set_style_bg_color(badge, lv_color_hex(0xEF4444), 0); 
            lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(badge, 2, 0);
            lv_obj_set_style_border_color(badge, lv_color_hex(0x05020A), 0);
            lv_obj_clear_flag(badge, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_scrollbar_mode(badge, LV_SCROLLBAR_MODE_OFF);

            lv_obj_t* badge_lbl = lv_label_create(badge);
            lv_label_set_text_fmt(badge_lbl, "%d", badge_count);
            lv_obj_set_style_text_color(badge_lbl, lv_color_hex(0xFFFFFF), 0);
            // Non potendo usare font diversi, usiamo font 14 anche qui, lo spazio di 26px lo contiene
            lv_obj_align(badge_lbl, LV_ALIGN_CENTER, 0, 0); 
        }

        if (show_label) {
            lv_obj_t* txt = lv_label_create(btn);
            lv_label_set_text(txt, name);
            lv_obj_set_style_text_color(txt, lv_color_hex(0xFFFFFF), 0);
            lv_obj_align(txt, LV_ALIGN_BOTTOM_MID, 0, 0);
        }

        return btn;
    }

public:
    HomeApp() : Application(0) {}
    
    void onStart() override {
        pending_app_id = -1;
        uptime_seconds = esp_timer_get_time() / 1000000ULL; 

        // SCREEN PRINCIPALE
        screen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x05020A), 0);
        lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x150A21), 0);
        lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
        lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);
        
        // MAGIA RESPONSIVE: Impostiamo lo schermo intero come una colonna Flex.
        // Gli oggetti si impileranno senza MAI sovrapporsi.
        lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(screen, 10, 0); // Margine globale
        lv_obj_set_style_pad_gap(screen, 15, 0); // Distanza fissa tra un blocco e l'altro

        // 1. STATUS BAR (Inizio colonna)
        lv_obj_t* status_bar = lv_obj_create(screen);
        lv_obj_set_size(status_bar, LV_PCT(100), 40); // 40px fissi in altezza
        lv_obj_set_style_bg_opa(status_bar, 0, 0);
        lv_obj_set_style_border_width(status_bar, 0, 0);
        lv_obj_set_style_pad_all(status_bar, 0, 0);
        lv_obj_set_scrollbar_mode(status_bar, LV_SCROLLBAR_MODE_OFF);

        time_lbl = lv_label_create(status_bar);
        lv_label_set_text(time_lbl, "10:30");
        lv_obj_set_style_text_color(time_lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(time_lbl, LV_ALIGN_LEFT_MID, 10, 0);

        lv_obj_t* status_icons = lv_obj_create(status_bar);
        lv_obj_set_size(status_icons, LV_SIZE_CONTENT, LV_PCT(100));
        lv_obj_align(status_icons, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_set_style_bg_opa(status_icons, 0, 0);
        lv_obj_set_style_border_width(status_icons, 0, 0);
        lv_obj_set_flex_flow(status_icons, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_gap(status_icons, 15, 0); // Più distanti su schermi larghi
        lv_obj_set_scrollbar_mode(status_icons, LV_SCROLLBAR_MODE_OFF);

        daas_icon = lv_label_create(status_icons);
        lv_label_set_text(daas_icon, LV_SYMBOL_SHUFFLE);
        lv_obj_set_style_text_color(daas_icon, lv_color_hex(0x10B981), 0);
        bt_icon = lv_label_create(status_icons);
        lv_label_set_text(bt_icon, LV_SYMBOL_BLUETOOTH);
        lv_obj_set_style_text_color(bt_icon, lv_color_hex(0x3B82F6), 0);
        wifi_icon = lv_label_create(status_icons);
        lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
        lv_obj_set_style_text_color(wifi_icon, lv_color_hex(0xA855F7), 0);


        // 2. HERO WIDGET
        lv_obj_t* hero = lv_obj_create(screen);
        lv_obj_set_size(hero, LV_PCT(95), 100); // 100px fissi in altezza
        lv_obj_set_style_bg_color(hero, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_bg_opa(hero, LV_OPA_90, 0);
        lv_obj_set_style_radius(hero, 16, 0);
        lv_obj_set_style_border_width(hero, 1, 0);
        lv_obj_set_style_border_color(hero, lv_color_hex(0x3B2C63), 0);
        lv_obj_set_scrollbar_mode(hero, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* greeting = lv_label_create(hero);
        lv_label_set_text(greeting, "Modular OS");
        lv_obj_set_style_text_color(greeting, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(greeting, LV_ALIGN_TOP_LEFT, 10, 10);

        uptime_lbl = lv_label_create(hero);
        lv_label_set_text(uptime_lbl, "Uptime: 0m");
        lv_obj_set_style_text_color(uptime_lbl, lv_color_hex(0x10B981), 0); 
        lv_obj_align(uptime_lbl, LV_ALIGN_BOTTOM_LEFT, 10, -10);

        lv_obj_t* opt_btn = lv_btn_create(hero);
        lv_obj_set_size(opt_btn, 110, 40);
        lv_obj_align(opt_btn, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_set_style_bg_color(opt_btn, lv_color_hex(0x3B82F6), 0);
        lv_obj_set_style_radius(opt_btn, 12, 0);
        lv_obj_set_style_border_width(opt_btn, 0, 0);
        
        lv_obj_t* opt_lbl = lv_label_create(opt_btn);
        lv_label_set_text(opt_lbl, "Clean RAM");
        lv_obj_set_style_text_color(opt_lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(opt_lbl);
        
        lv_obj_add_event_cb(opt_btn, [](lv_event_t* e){
            lv_obj_t* label = lv_obj_get_child(lv_event_get_target(e), 0);
            lv_label_set_text(label, "Done!");
        }, LV_EVENT_CLICKED, NULL);


        // 3. WORKSPACE (Spazio dinamico)
        lv_obj_t* workspace = lv_obj_create(screen);
        lv_obj_set_width(workspace, LV_PCT(95)); 
        // flex_grow(1) dice a LVGL: "Prendi TUTTO lo spazio verticale rimasto disponibile"
        // Così il workspace riempie lo schermo e schiaccia la dock in fondo.
        lv_obj_set_flex_grow(workspace, 1); 
        lv_obj_set_style_bg_opa(workspace, 0, 0);
        lv_obj_set_style_border_width(workspace, 0, 0);
        lv_obj_set_flex_flow(workspace, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(workspace, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_all(workspace, 0, 0);
        lv_obj_set_style_pad_row(workspace, 20, 0);
        lv_obj_set_scrollbar_mode(workspace, LV_SCROLLBAR_MODE_OFF);

        auto& apps = system->registry.getApps();
        for (auto& a : apps) {
            if(a.id == 1 || a.id == 2 || a.id == 99) continue;
            char symbol[2] = {a.name[0], '\0'}; 
            create_app_icon(workspace, a.id, a.name.c_str(), a.color, symbol, 0, true);
        }

        // 4. DOCK (Altezza fissa in fondo)
        lv_obj_t* dock = lv_obj_create(screen);
        lv_obj_set_size(dock, LV_PCT(95), 100); // 100px di altezza, abbastanza per le icone enormi
        lv_obj_set_style_bg_color(dock, lv_color_hex(0x2B1E4A), 0);
        lv_obj_set_style_bg_opa(dock, LV_OPA_90, 0);
        lv_obj_set_style_radius(dock, 24, 0);
        lv_obj_set_style_border_width(dock, 1, 0);
        lv_obj_set_style_border_color(dock, lv_color_hex(0x3B2C63), 0);
        lv_obj_set_flex_flow(dock, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(dock, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(dock, 0, 0);
        lv_obj_set_scrollbar_mode(dock, LV_SCROLLBAR_MODE_OFF);

        // Icone Dock (Senza testo, più larghe)
        create_app_icon(dock, 2, "Chat", 0x9333EA, LV_SYMBOL_BELL, 2, false); 
        create_app_icon(dock, 1, "Settings", 0x475569, LV_SYMBOL_SETTINGS, 0, false);
        create_app_icon(dock, 100, "Terminal", 0x10B981, LV_SYMBOL_DIRECTORY, 0, false);

        lv_scr_load(screen);
    }

    void onUpdate() override {
        if (lv_tick_get() - last_tick > 1000) { 
            uptime_seconds++;
            int m = uptime_seconds / 60;
            char up_str[32];
            snprintf(up_str, sizeof(up_str), "Uptime: %dm %lds", m, uptime_seconds % 60);
            if(uptime_lbl) lv_label_set_text(uptime_lbl, up_str);

            if(uptime_seconds % 60 == 0) {
                mock_minutes++;
                if(mock_minutes >= 60) { mock_minutes = 0; mock_hours++; }
                if(mock_hours >= 24) mock_hours = 0;
                char t_str[32];
                snprintf(t_str, sizeof(t_str), "%02d:%02d", mock_hours, mock_minutes);
                if(time_lbl) lv_label_set_text(time_lbl, t_str);
            }
            last_tick = lv_tick_get();
        }

        if (system->wifiConnected) lv_obj_clear_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(wifi_icon, LV_OBJ_FLAG_HIDDEN);

        if (system->btEnabled) lv_obj_clear_flag(bt_icon, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(bt_icon, LV_OBJ_FLAG_HIDDEN);

        if (system->daasConnected) lv_obj_clear_flag(daas_icon, LV_OBJ_FLAG_HIDDEN);
        else lv_obj_add_flag(daas_icon, LV_OBJ_FLAG_HIDDEN);
    }

    void onExit() override { if (screen) lv_obj_del(screen); screen = nullptr; }
    void onDraw() override {}
};