#pragma once
#include "os/interfaces/application_interface.hpp"
#include <vector>
#include <string>
#include "lvgl.h"

struct Contact { uint64_t din; std::string name; std::string last_msg; bool online; };

class MessengerApp : public Application {
private:
    lv_obj_t* screen = nullptr;
    lv_obj_t* list_p = nullptr;
    lv_obj_t* chat_p = nullptr;
    lv_obj_t* chat_list = nullptr;
    lv_obj_t* input_ta = nullptr;
    lv_obj_t* kb = nullptr;
    lv_obj_t* bottom_bar = nullptr;
    
    std::vector<Contact> contacts;

    void add_bubble(const char* text, bool mine) {
        // Riga invisibile per gestire l'allineamento a destra o sinistra
        lv_obj_t* row = lv_obj_create(chat_list);
        lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, 0, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);

        // Bolla vera e propria
        lv_obj_t* b = lv_obj_create(row);
        lv_obj_set_width(b, LV_SIZE_CONTENT);
        lv_obj_set_height(b, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(b, 12, 0);
        lv_obj_set_style_border_width(b, 0, 0);
        lv_obj_set_scrollbar_mode(b, LV_SCROLLBAR_MODE_OFF);
        
        if(mine) {
            lv_obj_set_style_bg_color(b, lv_color_hex(0x9333EA), 0); // Viola per messaggi inviati
            lv_obj_set_style_radius(b, 20, 0);
            lv_obj_align(b, LV_ALIGN_RIGHT_MID, -10, 0); // Margine dal bordo destro
        } else {
            lv_obj_set_style_bg_color(b, lv_color_hex(0x2B1E4A), 0); // Grigio scuro per messaggi ricevuti
            lv_obj_set_style_radius(b, 20, 0);
            lv_obj_align(b, LV_ALIGN_LEFT_MID, 10, 0); // Margine dal bordo sinistro
        }

        // Etichetta di testo
        lv_obj_t* l = lv_label_create(b);
        lv_label_set_text(l, text);
        lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP); // Manda a capo se sfora
        lv_obj_set_width(l, LV_SIZE_CONTENT);
        
        // FIX FONDAMENTALE: Fissare la larghezza massima in PIXEL impedisce il collasso (linea verticale)
        lv_obj_set_style_max_width(l, 250, 0); 
        
        lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0); // Testo Bianco leggibile
        
        // Forza lo scorrimento verso l'ultimo messaggio inviato
        lv_obj_scroll_to_view(row, LV_ANIM_ON);
    }

    static void kb_event_cb(lv_event_t* e) {
        MessengerApp* a = (MessengerApp*)lv_event_get_user_data(e);
        lv_event_code_t code = lv_event_get_code(e);
        
        if(code == LV_EVENT_READY) {
            // Premuto il tasto "Invio" / "V" sulla tastiera
            if(strlen(lv_textarea_get_text(a->input_ta)) > 0) {
                a->add_bubble(lv_textarea_get_text(a->input_ta), true);
                lv_textarea_set_text(a->input_ta, "");
            }
        } else if(code == LV_EVENT_CANCEL) {
            // Tasto per nascondere la tastiera
            lv_obj_add_flag(a->kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align(a->bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0); // Riporta giù la barra di input
            lv_obj_set_style_pad_bottom(a->chat_list, 70, 0); // Ripristina l'area visibile della chat
            lv_obj_clear_state(a->input_ta, LV_STATE_FOCUSED);
        }
    }

    static void ta_event_cb(lv_event_t* e) {
        MessengerApp* a = (MessengerApp*)lv_event_get_user_data(e);
        lv_event_code_t code = lv_event_get_code(e);
        
        if(code == LV_EVENT_FOCUSED) {
            // Quando tocchi la barra di testo, mostra la tastiera e alza la barra in modo matematico infallibile
            lv_obj_clear_flag(a->kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align(a->bottom_bar, LV_ALIGN_BOTTOM_MID, 0, -220); // Alza la barra sopra la tastiera (220px)
            lv_obj_set_style_pad_bottom(a->chat_list, 70 + 220, 0); // Restringe l'area di scorrimento chat
            
            // Scrolla all'ultimo messaggio
            if(lv_obj_get_child_cnt(a->chat_list) > 0) {
                lv_obj_scroll_to_view(lv_obj_get_child(a->chat_list, lv_obj_get_child_cnt(a->chat_list)-1), LV_ANIM_OFF);
            }
        }
    }

public:
    MessengerApp() : Application(2) {}

    void onStart() override {
        contacts = {
            {101, "Alice Freeman", "See you later!", true},
            {102, "Node-0X55", "Sync complete.", false},
            {103, "Bob", "Are we still on for tomorrow?", true}
        };

        screen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(screen, lv_color_hex(0x05020A), 0);
        lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);

        // ==========================================
        // 1. PAGINA LISTA CHAT
        // ==========================================
        list_p = lv_obj_create(screen);
        lv_obj_set_size(list_p, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_opa(list_p, 0, 0);
        lv_obj_set_style_border_width(list_p, 0, 0);
        lv_obj_set_flex_flow(list_p, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_scrollbar_mode(list_p, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* lh = lv_obj_create(list_p);
        lv_obj_set_size(lh, LV_PCT(100), 60);
        lv_obj_set_style_bg_color(lh, lv_color_hex(0x110A1F), 0);
        lv_obj_set_style_border_width(lh, 0, 0);
        lv_obj_set_scrollbar_mode(lh, LV_SCROLLBAR_MODE_OFF);

        lv_obj_t* btn_home = lv_btn_create(lh);
        lv_obj_set_size(btn_home, 45, 45);
        lv_obj_align(btn_home, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_set_style_bg_color(btn_home, lv_color_hex(0x2B1E4A), 0);
        lv_obj_set_style_radius(btn_home, LV_RADIUS_CIRCLE, 0);
        lv_obj_add_event_cb(btn_home, [](lv_event_t* e){ ((MessengerApp*)lv_event_get_user_data(e))->system->launchApp(0); }, LV_EVENT_CLICKED, this);
        lv_obj_t* l_home = lv_label_create(btn_home);
        lv_label_set_text(l_home, LV_SYMBOL_HOME);
        lv_obj_center(l_home);

        lv_obj_t* title_list = lv_label_create(lh);
        lv_label_set_text(title_list, "Messages");
        lv_obj_set_style_text_color(title_list, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(title_list, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t* clist = lv_obj_create(list_p);
        lv_obj_set_width(clist, LV_PCT(100));
        lv_obj_set_flex_grow(clist, 1); 
        lv_obj_set_style_bg_opa(clist, 0, 0);
        lv_obj_set_style_border_width(clist, 0, 0);
        lv_obj_set_flex_flow(clist, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(clist, 0, 0);
        lv_obj_set_scrollbar_mode(clist, LV_SCROLLBAR_MODE_OFF);

        for (auto& c : contacts) {
            lv_obj_t* btn = lv_btn_create(clist);
            lv_obj_set_size(btn, LV_PCT(100), 90);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x1B1433), 0);
            lv_obj_set_style_border_width(btn, 0, 0);
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_BOTTOM, 0);
            lv_obj_set_style_border_width(btn, 1, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x05020A), 0);
            lv_obj_set_scrollbar_mode(btn, LV_SCROLLBAR_MODE_OFF);
            
            lv_obj_t* av = lv_obj_create(btn);
            lv_obj_set_size(av, 55, 55);
            lv_obj_set_style_bg_color(av, lv_color_hex(0xA855F7), 0);
            lv_obj_set_style_radius(av, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(av, 0, 0);
            lv_obj_align(av, LV_ALIGN_LEFT_MID, 10, 0);
            lv_obj_set_scrollbar_mode(av, LV_SCROLLBAR_MODE_OFF);
            
            lv_obj_t* il = lv_label_create(av);
            lv_label_set_text(il, c.name.substr(0,1).c_str());
            lv_obj_set_style_text_color(il, lv_color_hex(0xFFFFFF), 0);
            lv_obj_center(il);

            if (c.online) {
                lv_obj_t* dot = lv_obj_create(btn);
                lv_obj_set_size(dot, 16, 16);
                lv_obj_align(dot, LV_ALIGN_LEFT_MID, 50, 20);
                lv_obj_set_style_bg_color(dot, lv_color_hex(0x10B981), 0);
                lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
                lv_obj_set_style_border_width(dot, 3, 0);
                lv_obj_set_style_border_color(dot, lv_color_hex(0x1B1433), 0);
                lv_obj_set_scrollbar_mode(dot, LV_SCROLLBAR_MODE_OFF);
            }

            lv_obj_t* nl = lv_label_create(btn);
            lv_label_set_text(nl, c.name.c_str());
            lv_obj_set_style_text_color(nl, lv_color_hex(0xFFFFFF), 0);
            lv_obj_align(nl, LV_ALIGN_TOP_LEFT, 80, 15);

            lv_obj_t* sl = lv_label_create(btn);
            lv_label_set_text(sl, c.last_msg.c_str());
            lv_obj_set_style_text_color(sl, lv_color_hex(0x8B7CA6), 0);
            lv_obj_align(sl, LV_ALIGN_BOTTOM_LEFT, 80, -15);

            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                MessengerApp* a = (MessengerApp*)lv_event_get_user_data(e);
                lv_obj_add_flag(a->list_p, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(a->chat_p, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clean(a->chat_list);
                a->add_bubble("Hey! This is a long message to test if the chat bubble wraps properly in LVGL without breaking into a vertical line.", false);
            }, LV_EVENT_CLICKED, this);
        }

        // ==========================================
        // 2. PAGINA CHAT ATTIVA (Layout Assoluto)
        // ==========================================
        chat_p = lv_obj_create(screen);
        lv_obj_set_size(chat_p, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_bg_opa(chat_p, 0, 0);
        lv_obj_set_style_border_width(chat_p, 0, 0);
        lv_obj_set_style_pad_all(chat_p, 0, 0);
        lv_obj_set_scrollbar_mode(chat_p, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_flag(chat_p, LV_OBJ_FLAG_HIDDEN);

        // Header Chat
        lv_obj_t* ch = lv_obj_create(chat_p);
        lv_obj_set_size(ch, LV_PCT(100), 60);
        lv_obj_align(ch, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_color(ch, lv_color_hex(0x110A1F), 0);
        lv_obj_set_style_border_width(ch, 0, 0);
        lv_obj_set_scrollbar_mode(ch, LV_SCROLLBAR_MODE_OFF);
        
        lv_obj_t* cbb = lv_btn_create(ch);
        lv_obj_set_size(cbb, 45, 45);
        lv_obj_align(cbb, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_set_style_bg_color(cbb, lv_color_hex(0x2B1E4A), 0);
        lv_obj_set_style_radius(cbb, LV_RADIUS_CIRCLE, 0);
        lv_obj_add_event_cb(cbb, [](lv_event_t* e){
            MessengerApp* a = (MessengerApp*)lv_event_get_user_data(e);
            lv_obj_add_flag(a->chat_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(a->list_p, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(a->kb, LV_OBJ_FLAG_HIDDEN); 
        }, LV_EVENT_CLICKED, this);
        lv_obj_t* cbbl = lv_label_create(cbb);
        lv_label_set_text(cbbl, LV_SYMBOL_LEFT);
        lv_obj_center(cbbl);

        lv_obj_t* contact_name = lv_label_create(ch);
        lv_label_set_text(contact_name, "Alice Freeman");
        lv_obj_set_style_text_color(contact_name, lv_color_hex(0xFFFFFF), 0);
        lv_obj_align(contact_name, LV_ALIGN_CENTER, 0, 0);

        // Area Scorrimento Messaggi
        chat_list = lv_obj_create(chat_p);
        lv_obj_set_size(chat_list, LV_PCT(100), LV_PCT(100)); 
        lv_obj_align(chat_list, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_bg_opa(chat_list, 0, 0);
        lv_obj_set_style_border_width(chat_list, 0, 0);
        lv_obj_set_flex_flow(chat_list, LV_FLEX_FLOW_COLUMN);
        
        // PAD TOP e BOTTOM per non andare sotto all'header e alla barra di input
        lv_obj_set_style_pad_top(chat_list, 60, 0); 
        lv_obj_set_style_pad_bottom(chat_list, 70, 0); 
        lv_obj_set_style_pad_left(chat_list, 5, 0);
        lv_obj_set_style_pad_right(chat_list, 5, 0);
        lv_obj_set_style_pad_gap(chat_list, 10, 0);

        // Barra di Input in basso
        bottom_bar = lv_obj_create(chat_p);
        lv_obj_set_size(bottom_bar, LV_PCT(100), 70);
        lv_obj_align(bottom_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(bottom_bar, lv_color_hex(0x1B1433), 0); // Visibilissima!
        lv_obj_set_style_border_width(bottom_bar, 0, 0);
        lv_obj_set_scrollbar_mode(bottom_bar, LV_SCROLLBAR_MODE_OFF);

        input_ta = lv_textarea_create(bottom_bar);
        lv_obj_set_size(input_ta, LV_PCT(78), 45);
        lv_obj_align(input_ta, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_set_style_bg_color(input_ta, lv_color_hex(0x05020A), 0); // Sfondo scurissimo per contrasto
        lv_obj_set_style_text_color(input_ta, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_width(input_ta, 1, 0);
        lv_obj_set_style_border_color(input_ta, lv_color_hex(0x3B2C63), 0);
        lv_obj_set_style_radius(input_ta, 22, 0);
        lv_textarea_set_placeholder_text(input_ta, "Type message...");
        lv_textarea_set_one_line(input_ta, true);
        lv_obj_set_scrollbar_mode(input_ta, LV_SCROLLBAR_MODE_OFF);
        
        lv_obj_add_event_cb(input_ta, ta_event_cb, LV_EVENT_ALL, this);

        lv_obj_t* send_btn = lv_btn_create(bottom_bar);
        lv_obj_set_size(send_btn, 45, 45);
        lv_obj_align(send_btn, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_set_style_bg_color(send_btn, lv_color_hex(0x9333EA), 0);
        lv_obj_set_style_radius(send_btn, LV_RADIUS_CIRCLE, 0);
        lv_obj_t* slbl = lv_label_create(send_btn);
        lv_label_set_text(slbl, LV_SYMBOL_PLAY);
        lv_obj_center(slbl);
        lv_obj_add_event_cb(send_btn, [](lv_event_t* e){
            MessengerApp* a = (MessengerApp*)lv_event_get_user_data(e);
            if(strlen(lv_textarea_get_text(a->input_ta)) > 0) {
                a->add_bubble(lv_textarea_get_text(a->input_ta), true);
                lv_textarea_set_text(a->input_ta, "");
            }
        }, LV_EVENT_CLICKED, this);


        // ==========================================
        // 3. TASTIERA GLOBALE (Fuori dal flex!)
        // ==========================================
        kb = lv_keyboard_create(screen);
        lv_keyboard_set_textarea(kb, input_ta);
        lv_obj_set_size(kb, LV_PCT(100), 220); 
        lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
        
        // Tema Tastiera Scuro
        lv_obj_set_style_bg_color(kb, lv_color_hex(0x05020A), LV_PART_MAIN);
        lv_obj_set_style_border_width(kb, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_color(kb, lv_color_hex(0x2B1E4A), LV_PART_ITEMS);
        lv_obj_set_style_text_color(kb, lv_color_hex(0xFFFFFF), LV_PART_ITEMS);
        lv_obj_set_style_radius(kb, 8, LV_PART_ITEMS);
        lv_obj_set_style_border_width(kb, 0, LV_PART_ITEMS);
        
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN); // Inizialmente nascosta
        lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, this);

        lv_scr_load(screen);
    }
    void onUpdate() override {}
    void onExit() override { if (screen) lv_obj_del(screen); screen = nullptr; }
    void onDraw() override {}
};