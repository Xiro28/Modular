#pragma once
#include "os/interfaces/application_interface.hpp"
#include <string>
#include <vector>
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "lvgl.h"

class SettingsApp : public Application {
private:
    lv_obj_t* screen = nullptr;
    lv_obj_t* content_area = nullptr;
    
    lv_obj_t* main_p = nullptr;
    lv_obj_t* wifi_p = nullptr;
    lv_obj_t* bt_p = nullptr;
    lv_obj_t* daas_p = nullptr;
    lv_obj_t* stat_p = nullptr;
    
    lv_obj_t* dot_wifi = nullptr;
    lv_obj_t* dot_bt = nullptr;
    lv_obj_t* dot_daas = nullptr;
    
    lv_obj_t* stat_lbl = nullptr;
    lv_chart_series_t* ram_series = nullptr;
    lv_obj_t* ram_chart = nullptr;
    lv_obj_t* wifi_list = nullptr;
    lv_obj_t* bt_list = nullptr;
    
    uint32_t last_tick = 0;

    // --- FUNZIONI HARDWARE REALI ---

    std::string get_wifi_ip() {
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if(netif) {
            esp_netif_ip_info_t ip_info;
            if(esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
                char str[16];
                esp_ip4addr_ntoa(&ip_info.ip, str, 16);
                if(std::string(str) != "0.0.0.0") return std::string(str);
            }
        }
        return "Not Connected";
    }

    std::string get_bt_mac() {
        uint8_t mac[6];
        if (esp_read_mac(mac, ESP_MAC_BT) == ESP_OK) {
            char str[20];
            snprintf(str, sizeof(str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            return std::string(str);
        }
        return "Unknown MAC";
    }

    static void sys_enable_daas_driver(const char* type, const char* address) {
        // Questa chiamata deve finire nel tuo modulo DaaS
        printf("\n[DaaS KERNEL] -> Abilitazione Driver: %s\n", type);
        printf("[DaaS KERNEL] -> Indirizzo Registrato: %s\n\n", address);
    }

    static void back_cb(lv_event_t* e) {
        SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
        if (!lv_obj_has_flag(app->main_p, LV_OBJ_FLAG_HIDDEN)) {
            app->system->launchApp(0);
        } else {
            lv_obj_add_flag(app->wifi_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(app->bt_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(app->daas_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(app->stat_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(app->main_p, LV_OBJ_FLAG_HIDDEN);
            app->refresh_lines();
        }
    }

    lv_obj_t* create_card(lv_obj_t* parent, const char* icon, const char* title, const char* subtitle, lv_obj_t** dot_ptr) {
        lv_obj_t* card = lv_btn_create(parent);
        lv_obj_set_size(card, LV_PCT(100), 80);
        lv_obj_set_style_bg_color(card, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_radius(card, 16, 0);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_border_color(card, lv_color_hex(0x2B1E4A), 0);
        lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
        
        lv_obj_t* icn_bg = lv_obj_create(card);
        lv_obj_set_size(icn_bg, 50, 50);
        lv_obj_set_style_bg_color(icn_bg, lv_color_hex(0x2B1E4A), 0);
        lv_obj_set_style_radius(icn_bg, 12, 0);
        lv_obj_set_style_border_width(icn_bg, 0, 0);
        lv_obj_align(icn_bg, LV_ALIGN_LEFT_MID, 5, 0);
        
        lv_obj_t* icn = lv_label_create(icn_bg);
        lv_label_set_text(icn, icon);
        lv_obj_set_style_text_color(icn, lv_color_hex(0xA855F7), 0);
        lv_obj_center(icn);

        lv_obj_t* t = lv_label_create(card);
        lv_label_set_text(t, title);
        lv_obj_set_style_text_color(t, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(t, LV_ALIGN_LEFT_MID, 70, -10);

        lv_obj_t* st = lv_label_create(card);
        lv_label_set_text(st, subtitle);
        lv_obj_set_style_text_color(st, lv_color_hex(0xA192BB), 0);
        lv_obj_align(st, LV_ALIGN_LEFT_MID, 70, 15);

        if (dot_ptr) {
            *dot_ptr = lv_obj_create(card);
            lv_obj_set_size(*dot_ptr, 14, 14);
            lv_obj_align(*dot_ptr, LV_ALIGN_RIGHT_MID, -15, 0);
            lv_obj_set_style_bg_color(*dot_ptr, lv_color_hex(0xEF4444), 0);
            lv_obj_set_style_radius(*dot_ptr, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(*dot_ptr, 2, 0);
            lv_obj_set_style_border_color(*dot_ptr, lv_color_hex(0x1B1433), 0);
        }
        return card;
    }

    void refresh_lines() {
        if(dot_wifi) lv_obj_set_style_bg_color(dot_wifi, lv_color_hex(system->wifiConnected ? 0x10B981 : 0xEF4444), 0);
        if(dot_bt) lv_obj_set_style_bg_color(dot_bt, lv_color_hex(system->btEnabled ? 0x10B981 : 0xEF4444), 0);
        if(dot_daas) lv_obj_set_style_bg_color(dot_daas, lv_color_hex(system->daasConnected ? 0x10B981 : 0xEF4444), 0);
    }

    lv_obj_t* create_sub_page() {
        lv_obj_t* p = lv_obj_create(content_area);
        lv_obj_set_size(p, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_opa(p, 0, 0);
        lv_obj_set_style_border_width(p, 0, 0);
        lv_obj_set_style_pad_all(p, 0, 0);
        lv_obj_set_flex_flow(p, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_gap(p, 10, 0);
        lv_obj_set_scrollbar_mode(p, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);
        return p;
    }

    // --- COSTRUZIONE SOTTOMENU CON DATI REALI ---

    void build_wifi_page() {
        lv_obj_t* header = lv_obj_create(wifi_p);
        lv_obj_set_size(header, LV_PCT(100), 75);
        lv_obj_set_style_bg_color(header, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_radius(header, 16, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* lbl = lv_label_create(header);
        lv_label_set_text(lbl, "Wi-Fi Power");
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 15, 10);

        lv_obj_t* ip_lbl = lv_label_create(header);
        std::string ip = "IP: " + get_wifi_ip();
        lv_label_set_text(ip_lbl, ip.c_str());
        lv_obj_set_style_text_color(ip_lbl, lv_color_hex(0x10B981), 0);
        lv_obj_align(ip_lbl, LV_ALIGN_BOTTOM_LEFT, 15, -10);

        lv_obj_t* sw = lv_switch_create(header);
        lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -15, 0);
        if(system->wifiConnected) lv_obj_add_state(sw, LV_STATE_CHECKED);
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x10B981), LV_PART_INDICATOR | LV_STATE_CHECKED);

        lv_obj_t* scan_btn = lv_btn_create(wifi_p);
        lv_obj_set_size(scan_btn, LV_PCT(100), 50);
        lv_obj_set_style_bg_color(scan_btn, lv_color_hex(0x3B82F6), 0);
        lv_obj_set_style_radius(scan_btn, 12, 0);
        lv_obj_t* scan_lbl = lv_label_create(scan_btn);
        lv_label_set_text(scan_lbl, LV_SYMBOL_REFRESH " Scan Networks");
        lv_obj_center(scan_lbl);

        wifi_list = lv_obj_create(wifi_p);
        lv_obj_set_width(wifi_list, LV_PCT(100));
        lv_obj_set_flex_grow(wifi_list, 1);
        lv_obj_set_style_bg_opa(wifi_list, 0, 0);
        lv_obj_set_style_border_width(wifi_list, 0, 0);
        lv_obj_set_flex_flow(wifi_list, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(wifi_list, 0, 0);
        lv_obj_set_scrollbar_mode(wifi_list, LV_SCROLLBAR_MODE_OFF);

        // EVENTO SCANSIONE REALE
        lv_obj_add_event_cb(scan_btn, [](lv_event_t* e){
            SettingsApp* a = (SettingsApp*)lv_event_get_user_data(e);
            lv_obj_clean(a->wifi_list);
            
            lv_obj_t* loading = lv_label_create(a->wifi_list);
            lv_label_set_text(loading, "Scanning... (Real Data)");
            lv_obj_set_style_text_color(loading, lv_color_hex(0x8B7CA6), 0);
            lv_obj_align(loading, LV_ALIGN_CENTER, 0, 20);

            // Avvio scansione REALE asincrona
            wifi_scan_config_t scan_config = {};
            scan_config.ssid = 0;
            scan_config.bssid = 0;
            scan_config.channel = 0;
            scan_config.show_hidden = false;
            
            esp_err_t err = esp_wifi_scan_start(&scan_config, false);

            if (err != ESP_OK) {
                lv_label_set_text_fmt(loading, "Wi-Fi Error: %d", err);
                return;
            }

            // Polling asincrono sui risultati
            lv_timer_create([](lv_timer_t* t){
                SettingsApp* app = (SettingsApp*)t->user_data;
                uint16_t ap_count = 0;
                
                esp_err_t ret = esp_wifi_scan_get_ap_num(&ap_count);
                
                if (ret == ESP_OK) {
                    lv_obj_clean(app->wifi_list);
                    
                    if (ap_count == 0) {
                        lv_obj_t* empty = lv_label_create(app->wifi_list);
                        lv_label_set_text(empty, "No networks found.");
                        lv_obj_align(empty, LV_ALIGN_CENTER, 0, 20);
                    } else {
                        uint16_t max_aps = (ap_count > 15) ? 15 : ap_count;
                        wifi_ap_record_t* ap_records = new wifi_ap_record_t[max_aps];
                        
                        if (esp_wifi_scan_get_ap_records(&max_aps, ap_records) == ESP_OK) {
                            for(int i = 0; i < max_aps; i++) {
                                lv_obj_t* net = lv_btn_create(app->wifi_list);
                                lv_obj_set_size(net, LV_PCT(100), 60);
                                lv_obj_set_style_bg_color(net, lv_color_hex(0x150A21), 0);
                                lv_obj_set_style_border_width(net, 0, 0);
                                
                                lv_obj_t* nl = lv_label_create(net);
                                lv_label_set_text(nl, (char*)ap_records[i].ssid);
                                lv_obj_align(nl, LV_ALIGN_LEFT_MID, 10, 0);
                                
                                lv_obj_t* ic = lv_label_create(net);
                                char rssi_str[32];
                                snprintf(rssi_str, sizeof(rssi_str), "%d dBm " LV_SYMBOL_WIFI, ap_records[i].rssi);
                                lv_label_set_text(ic, rssi_str);
                                lv_obj_set_style_text_color(ic, lv_color_hex(0x8B7CA6), 0);
                                lv_obj_align(ic, LV_ALIGN_RIGHT_MID, -10, 0);
                            }
                        }
                        delete[] ap_records;
                    }
                    lv_timer_del(t); // Fine scansione, uccidi il timer
                } else if (ret != ESP_ERR_WIFI_STATE) {
                    lv_obj_clean(app->wifi_list);
                    lv_obj_t* err_lbl = lv_label_create(app->wifi_list);
                    lv_label_set_text(err_lbl, "Scan failed.");
                    lv_timer_del(t);
                }
            }, 500, a); 
        }, LV_EVENT_CLICKED, this);
    }

    void build_bt_page() {
        lv_obj_t* header = lv_obj_create(bt_p);
        lv_obj_set_size(header, LV_PCT(100), 75);
        lv_obj_set_style_bg_color(header, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_radius(header, 16, 0);
        lv_obj_set_style_border_width(header, 0, 0);
        lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* lbl = lv_label_create(header);
        lv_label_set_text(lbl, "Bluetooth Power");
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 15, 10);

        lv_obj_t* mac_lbl = lv_label_create(header);
        std::string mac = "MAC: " + get_bt_mac();
        lv_label_set_text(mac_lbl, mac.c_str());
        lv_obj_set_style_text_color(mac_lbl, lv_color_hex(0x8B7CA6), 0);
        lv_obj_align(mac_lbl, LV_ALIGN_BOTTOM_LEFT, 15, -10);

        lv_obj_t* sw = lv_switch_create(header);
        lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -15, 0);
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x3B82F6), LV_PART_INDICATOR | LV_STATE_CHECKED);

        lv_obj_t* vis_btn = lv_btn_create(bt_p);
        lv_obj_set_size(vis_btn, LV_PCT(100), 50);
        lv_obj_set_style_bg_color(vis_btn, lv_color_hex(0x9333EA), 0);
        lv_obj_set_style_radius(vis_btn, 12, 0);
        lv_obj_t* vis_lbl = lv_label_create(vis_btn);
        lv_label_set_text(vis_lbl, LV_SYMBOL_EYE_OPEN " Make Discoverable");
        lv_obj_center(vis_lbl);

        lv_obj_t* title = lv_label_create(bt_p);
        lv_label_set_text(title, "Bluetooth functions require GAP init");
        lv_obj_set_style_text_color(title, lv_color_hex(0x8B7CA6), 0);
    }
    
    void build_daas_page() {
        lv_obj_t* title = lv_label_create(daas_p);
        lv_label_set_text(title, "DaaS Driver Config");
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);

        lv_obj_t* d_wifi = lv_btn_create(daas_p);
        lv_obj_set_size(d_wifi, LV_PCT(100), 70);
        lv_obj_set_style_bg_color(d_wifi, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_radius(d_wifi, 16, 0);
        lv_obj_t* lw = lv_label_create(d_wifi);
        lv_label_set_text(lw, "Enable Driver Wi-Fi");
        lv_obj_align(lw, LV_ALIGN_LEFT_MID, 10, 0);
        
        lv_obj_add_event_cb(d_wifi, [](lv_event_t* e){
            SettingsApp* a = (SettingsApp*)lv_event_get_user_data(e);
            std::string ip_port = a->get_wifi_ip() + ":" + std::to_string(random() % 1000 + 8000);
            sys_enable_daas_driver("WIFI", ip_port.c_str());
            lv_obj_set_style_bg_color(lv_event_get_target(e), lv_color_hex(0x10B981), 0);
        }, LV_EVENT_CLICKED, this);

        lv_obj_t* d_bt = lv_btn_create(daas_p);
        lv_obj_set_size(d_bt, LV_PCT(100), 70);
        lv_obj_set_style_bg_color(d_bt, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_radius(d_bt, 16, 0);
        lv_obj_t* lb = lv_label_create(d_bt);
        lv_label_set_text(lb, "Enable Driver Bluetooth");
        lv_obj_align(lb, LV_ALIGN_LEFT_MID, 10, 0);
        
        lv_obj_add_event_cb(d_bt, [](lv_event_t* e){
            SettingsApp* a = (SettingsApp*)lv_event_get_user_data(e);
            std::string mac = a->get_bt_mac();
            sys_enable_daas_driver("BLUETOOTH", mac.c_str());
            lv_obj_set_style_bg_color(lv_event_get_target(e), lv_color_hex(0x3B82F6), 0);
        }, LV_EVENT_CLICKED, this);
    }

    void build_stats_page() {
        lv_obj_t* arc_ctn = lv_obj_create(stat_p);
        lv_obj_set_size(arc_ctn, LV_PCT(100), 160);
        lv_obj_set_style_bg_color(arc_ctn, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_border_width(arc_ctn, 0, 0);
        lv_obj_set_style_radius(arc_ctn, 16, 0);

        lv_obj_t* arc = lv_arc_create(arc_ctn);
        lv_obj_set_size(arc, 130, 130);
        lv_arc_set_rotation(arc, 270);
        lv_arc_set_bg_angles(arc, 0, 360);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_center(arc);
        lv_obj_set_style_arc_color(arc, lv_color_hex(0x2B1E4A), LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc, lv_color_hex(0x10B981), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(arc, 15, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc, 15, LV_PART_INDICATOR);
        lv_arc_set_value(arc, 50);

        stat_lbl = lv_label_create(arc);
        lv_label_set_text(stat_lbl, "RAM");
        lv_obj_set_style_text_color(stat_lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(stat_lbl);

        ram_chart = lv_chart_create(stat_p);
        lv_obj_set_width(ram_chart, LV_PCT(100));
        lv_obj_set_flex_grow(ram_chart, 1); 
        lv_obj_set_style_bg_color(ram_chart, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_border_width(ram_chart, 0, 0);
        lv_obj_set_style_radius(ram_chart, 16, 0);
        
        lv_chart_set_type(ram_chart, LV_CHART_TYPE_LINE);
        lv_obj_set_style_line_width(ram_chart, 3, LV_PART_ITEMS);
        lv_obj_set_style_width(ram_chart, 0, LV_PART_INDICATOR);
        lv_obj_set_style_height(ram_chart, 0, LV_PART_INDICATOR);
        
        ram_series = lv_chart_add_series(ram_chart, lv_color_hex(0x9333EA), LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_point_count(ram_chart, 20);
        lv_chart_set_update_mode(ram_chart, LV_CHART_UPDATE_MODE_SHIFT);
        lv_obj_set_style_bg_opa(ram_chart, LV_OPA_30, LV_PART_ITEMS); 
        lv_obj_set_style_bg_color(ram_chart, lv_color_hex(0x9333EA), LV_PART_ITEMS);
    }

public:
    SettingsApp() : Application(1) {}
    
    void onStart() override {
        screen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x05020A), 0);
        lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);

        lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(screen, 10, 0);
        lv_obj_set_style_pad_gap(screen, 10, 0);

        lv_obj_t* head = lv_obj_create(screen);
        lv_obj_set_size(head, LV_PCT(100), 50);
        lv_obj_set_style_bg_opa(head, 0, 0);
        lv_obj_set_style_border_width(head, 0, 0);
        lv_obj_set_scrollbar_mode(head, LV_SCROLLBAR_MODE_OFF);
        
        lv_obj_t* back = lv_btn_create(head);
        lv_obj_set_size(back, 50, 50);
        lv_obj_align(back, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_bg_color(back, lv_color_hex(0x1B1433), 0);
        lv_obj_set_style_radius(back, LV_RADIUS_CIRCLE, 0);
        lv_obj_add_event_cb(back, back_cb, LV_EVENT_CLICKED, this);
        lv_obj_t* back_lbl = lv_label_create(back);
        lv_label_set_text(back_lbl, LV_SYMBOL_LEFT);
        lv_obj_center(back_lbl);

        lv_obj_t* title = lv_label_create(head);
        lv_label_set_text(title, "Settings");
        lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

        content_area = lv_obj_create(screen);
        lv_obj_set_width(content_area, LV_PCT(100));
        lv_obj_set_flex_grow(content_area, 1); 
        lv_obj_set_style_bg_opa(content_area, 0, 0);
        lv_obj_set_style_border_width(content_area, 0, 0);
        lv_obj_set_scrollbar_mode(content_area, LV_SCROLLBAR_MODE_OFF);

        main_p = lv_obj_create(content_area);
        lv_obj_set_size(main_p, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_opa(main_p, 0, 0);
        lv_obj_set_style_border_width(main_p, 0, 0);
        lv_obj_set_flex_flow(main_p, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_gap(main_p, 15, 0);
        lv_obj_set_scrollbar_mode(main_p, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* c1 = create_card(main_p, LV_SYMBOL_WIFI, "Wi-Fi", "Networking & Scans", &dot_wifi);
        lv_obj_add_event_cb(c1, [](lv_event_t* e){
            SettingsApp* a = (SettingsApp*)lv_event_get_user_data(e);
            lv_obj_add_flag(a->main_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(a->wifi_p, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, this);

        lv_obj_t* c2 = create_card(main_p, LV_SYMBOL_BLUETOOTH, "Bluetooth", "Discover & Pair", &dot_bt);
        lv_obj_add_event_cb(c2, [](lv_event_t* e){
            SettingsApp* a = (SettingsApp*)lv_event_get_user_data(e);
            lv_obj_add_flag(a->main_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(a->bt_p, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, this);
        
        lv_obj_t* c3 = create_card(main_p, LV_SYMBOL_SHUFFLE, "DaaS Node", "Driver Config", &dot_daas);
        lv_obj_add_event_cb(c3, [](lv_event_t* e){
            SettingsApp* a = (SettingsApp*)lv_event_get_user_data(e);
            lv_obj_add_flag(a->main_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(a->daas_p, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, this);

        lv_obj_t* c4 = create_card(main_p, LV_SYMBOL_LIST, "System Monitor", "RAM & Performance", nullptr);
        lv_obj_add_event_cb(c4, [](lv_event_t* e){
            SettingsApp* a = (SettingsApp*)lv_event_get_user_data(e);
            lv_obj_add_flag(a->main_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(a->stat_p, LV_OBJ_FLAG_HIDDEN);
        }, LV_EVENT_CLICKED, this);

        wifi_p = create_sub_page(); build_wifi_page();
        bt_p = create_sub_page();   build_bt_page();
        daas_p = create_sub_page(); build_daas_page();
        stat_p = create_sub_page(); build_stats_page();

        refresh_lines();
        lv_scr_load(screen);
    }

    void onUpdate() override {
        if (lv_tick_get() - last_tick > 1000) {
            if (ram_chart && ram_series && !lv_obj_has_flag(stat_p, LV_OBJ_FLAG_HIDDEN)) {
                uint32_t free_kb = esp_get_free_heap_size() / 1024;
                lv_chart_set_next_value(ram_chart, ram_series, free_kb);
                std::string s = std::to_string(free_kb / 1024) + "MB\nFree";
                if(stat_lbl) lv_label_set_text(stat_lbl, s.c_str());
            }
            last_tick = lv_tick_get();
        }
    }
    
    void onExit() override { if (screen) lv_obj_del(screen); screen = nullptr; }
    void onDraw() override {}
};