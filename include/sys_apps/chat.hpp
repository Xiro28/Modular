#pragma once
#include "os/interfaces/application_interface.hpp"
#include <vector>
#include <string>

#include "daas/daas.hpp"

// Strutture Dati
struct Contact {
    uint64_t din;
    String lastMsg;
    bool isOnline;
    uint16_t color; // Colore avatar
};

struct Message {
    String text;
    bool isMine; // true = inviato da me, false = ricevuto
    stime_t timestamp;
};

// Stati dell'App
enum MsgState {
    MSG_CONTACTS,
    MSG_CHAT,
    MSG_KEYBOARD
};

class MessengerApp : public Application {
private:
    MsgState state = MSG_CONTACTS;
    bool needsRedraw = true;
    
    // Dati
    std::vector<Contact> contacts;
    std::vector<Message> currentChat; // Messaggi della chat aperta
    
    int selectedContactIdx = -1;
    int scrollY = 0; // Per lo scorrimento della lista contatti
    
    // Simulazione risposta automatica
    stime_t lastMsgTime = 0;
    bool waitingReply = false;

    // Layout Costanti
    const int ROW_H = 70; // Altezza riga contatto
    const int INPUT_H = 50; // Altezza barra input

    DaasAPI* getDaas() {
        return system->getNode();
    }
public:
    MessengerApp() : Application(2) {} // ID arbitrario 2


    void updateContactList() {
        contacts.clear();
        auto nodes = system->getNode()->listNodes();
        for (u32_t idx = 0; idx < nodes.size(); idx++) {
            contacts.push_back({nodes[idx], "Ciao!", true, 0x07E0}); // Verde
        }
    }

    void onStart() override {
        state = MSG_CONTACTS;
        needsRedraw = true;
        
        /*
        if (contacts.empty()) {
            contacts.push_back({101, "Alice", "Hey, come va?", true, 0xE46C}); // Orange
            contacts.push_back({102, "Bob Tech", "Il server è up.", false, 0x04F9}); // Blue
            contacts.push_back({103, "Charlie", "Ok, a dopo!", true, 0x9492}); // Gray
            contacts.push_back({104, "DaaS Node", "Sync completato.", true, 0xF800}); // Red
            contacts.push_back({105, "Eve", "Non credo...", false, 0x9000});
        }
            */
    }

    void onUpdate() override {
        // Gestione Risposta Automatica (Bot)
        uint32_t pulls = 0;
        if (state == MSG_CHAT) {

            DDO* ddo;

            if (system->getNode()->pull(contacts[selectedContactIdx].din, &ddo) == ERROR_NONE) {
                char* buffer = new char[ddo->getPayloadSize() + 1];
                memcpy(buffer, ddo->getPayloadPtr(), ddo->getPayloadSize());
                buffer[ddo->getPayloadSize()] = '\0'; 

                currentChat.push_back({String(buffer), false, millis()});
                contacts[selectedContactIdx].lastMsg = String(buffer);

                delete[] buffer;
                delete ddo;
                needsRedraw = true;
            }
        }

        switch (state) {
            case MSG_CONTACTS:
                if (needsRedraw) { drawContactList(); needsRedraw = false; }
                handleContactsTouch();
                break;

            case MSG_CHAT:
                if (needsRedraw) { drawChatInterface(); needsRedraw = false; }
                handleChatTouch();
                break;

            case MSG_KEYBOARD:
                // Gestione logica tastiera
                auto kb = system->getKeyboard();
                if (needsRedraw) { 
                    kb->begin("Scrivi a " + String(contacts[selectedContactIdx].din));
                    needsRedraw = false; 
                }
                
                kb->update();

                if (kb->isDone()) {
                    if (!kb->wasCancelled()) {
                        sendMessage(kb->getResult());
                    }
                    state = MSG_CHAT; // Torna alla chat
                    needsRedraw = true;
                }
                break;
        }
    }

    void onExit() override { }
    void onDraw() override {
        updateContactList();
     }

private:
    // --- LOGICA LISTA CONTATTI ---

    void drawContactList() {
        // Header
        hw->tft.fillScreen(theme->BG_COLOR);
        drawHeader("MESSAGES");

        int listStart = 50;
        int w = hw->tft.width();

        // Disegna lista contatti
        for (int i = 0; i < contacts.size(); i++) {
            int y = listStart + (i * ROW_H) - scrollY;
            
            // Ottimizzazione: Disegna solo se visibile nello schermo
            if (y + ROW_H < 50 || y > hw->tft.height()) continue;

            // 1. Riga Sfondo (Cliccabile)
            hw->tft.drawLine(20, y + ROW_H - 1, w - 20, y + ROW_H - 1, theme->PANEL_SHADOW);

            // 2. Avatar (Cerchio con iniziale)
            int avR = 22; // Raggio avatar
            int avX = 35;
            int avY = y + ROW_H/2;
            
            hw->tft.fillCircle(avX, avY, avR, contacts[i].color);
            hw->tft.setTextColor(theme->TEXT_MAIN, contacts[i].color);
            hw->tft.setTextDatum(textdatum_t::middle_center);
            hw->tft.setFont(&fonts::efontCN_14);
            String initial = String(contacts[i].din).substring(0, 1);
            hw->tft.drawString(initial, avX, avY);

            // Pallino Online
            if (contacts[i].isOnline) {
                hw->tft.fillCircle(avX + 15, avY + 15, 6, theme->BG_COLOR); // Bordo
                hw->tft.fillCircle(avX + 15, avY + 15, 4, 0x07E0); // Verde (Online)
            }

            // 3. Testi
            int textX = 70;
            hw->tft.setTextDatum(textdatum_t::top_left);
            
            // Nome
            hw->tft.setTextColor(theme->TEXT_MAIN, theme->BG_COLOR);
            hw->tft.drawString(String(contacts[i].din), textX, y + 15);
            
            // Ultimo Messaggio (Grigio e troncato)
            hw->tft.setTextColor(theme->TEXT_MUTED, theme->BG_COLOR);
            String msg = contacts[i].lastMsg;
            if (msg.length() > 20) msg = msg.substring(0, 19) + "...";
            hw->tft.drawString(msg, textX, y + 40);

            // Orario finto (a destra)
           /* hw->tft.setTextDatum(textdatum_t::top_right);
            hw->tft.drawString("10:30", w - 10, y + 15);*/
        }
    }

    void handleContactsTouch() {
        if (!hw->isTouching) return;
        delay(150);

        // Header Back
        if (hw->isTouchInRect(0, 0, 50, 50)) {
            system->launchApp(0); // Home
            return;
        }

        // Click su contatto
        int yTouch = hw->touchY;
        int listStart = 50;
        
        // Calcolo indice basato su Y e Scroll
        // (y - start + scroll) / altezza_riga
        int idx = (yTouch - listStart + scrollY) / ROW_H;

        if (idx >= 0 && idx < contacts.size()) {
            openChat(idx);
        }
    }

    // --- LOGICA CHAT INTERFACE ---

    void openChat(int idx) {
        selectedContactIdx = idx;
        state = MSG_CHAT;
        needsRedraw = true;
        
        // Pulisce o carica messaggi precedenti (qui simulo reset o storico finto)
        currentChat.clear();
        // Aggiungi un messaggio di benvenuto finto
        currentChat.push_back({contacts[idx].lastMsg, false, millis()});
    }

    void drawChatInterface() {
        hw->tft.fillScreen(theme->BG_COLOR);
        
        // 1. Header Chat (Nome contatto + Back)
        int w = hw->tft.width();
        hw->tft.fillRect(0, 0, w, 50, theme->HEADER_BG);
        hw->tft.drawFastHLine(0, 50, w, theme->PANEL_SHADOW);
        
        hw->tft.setTextColor(theme->ACCENT_WARN, theme->HEADER_BG);
        hw->tft.drawString("<", 10, 15); // Back icon
        
        // Avatar piccolo header
        hw->tft.fillCircle(40, 25, 15, contacts[selectedContactIdx].color);
        
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->HEADER_BG);
        hw->tft.setTextDatum(textdatum_t::middle_left);
        hw->tft.drawString(String(contacts[selectedContactIdx].din), 65, 25);
        
        // 2. Area Messaggi
        int chatBottom = hw->tft.height() - INPUT_H;
        int y = 60; // Start Y messaggi

        for (const auto& msg : currentChat) {
            drawMessageBubble(msg, y);
            y += 45; // Spazio fisso per semplicità (in una vera app calcoli altezza testo)
        }

        // 3. Barra Input (in basso)
        int barY = hw->tft.height() - INPUT_H;
        hw->tft.fillRect(0, barY, w, INPUT_H, theme->PANEL_BG);
        hw->tft.drawFastHLine(0, barY, w, theme->BORDER_COLOR);
        
        // Finto box di testo
        hw->tft.fillRoundRect(10, barY + 8, w - 60, 34, 17, theme->BG_COLOR);
        hw->tft.setTextColor(theme->TEXT_MUTED, theme->BG_COLOR);
        hw->tft.drawString("Message...", 20, barY + 24);
        
        // Pulsante invio (icona o cerchio)
        hw->tft.fillCircle(w - 25, barY + 25, 18, theme->ACCENT_PRIMARY);
        hw->tft.setTextColor(theme->TEXT_MAIN, theme->ACCENT_PRIMARY);
        hw->tft.setTextDatum(textdatum_t::middle_center);
        hw->tft.drawString(">", w - 25, barY + 25);
    }

    void drawMessageBubble(const Message& msg, int y) {
        int w = hw->tft.width();
        int maxBubbleW = w * 0.7; // Max 70% larghezza schermo
        
        // Calcola larghezza testo (approssimata per semplicità)
        int txtW = hw->tft.textWidth(msg.text);
        int bubbleW = txtW + 20; 
        if (bubbleW > maxBubbleW) bubbleW = maxBubbleW; // Clamp
        
        int bubbleH = 35; // Altezza fissa (per ora)

        if (msg.isMine) {
            // I miei messaggi (Destra, Blu)
            int x = w - bubbleW - 10;
            hw->tft.fillRoundRect(x, y, bubbleW, bubbleH, 12, theme->ACCENT_PRIMARY);
            // "Coda" della bolla
            hw->tft.fillTriangle(w-15, y+bubbleH-5, w-5, y+bubbleH, w-15, y+bubbleH, theme->ACCENT_PRIMARY);

            hw->tft.setTextColor(theme->TEXT_MAIN, theme->ACCENT_PRIMARY);
            hw->tft.setTextDatum(textdatum_t::middle_right);
            hw->tft.drawString(msg.text, x + bubbleW - 10, y + bubbleH/2);
        } else {
            // Messaggi altri (Sinistra, Grigio scuro)
            int x = 10;
            hw->tft.fillRoundRect(x, y, bubbleW, bubbleH, 12, theme->PANEL_BG);
            // "Coda"
            hw->tft.fillTriangle(x+5, y+bubbleH, x+15, y+bubbleH, x+5, y+bubbleH-5, theme->PANEL_BG);

            hw->tft.setTextColor(theme->TEXT_MAIN, theme->PANEL_BG);
            hw->tft.setTextDatum(textdatum_t::middle_left);
            hw->tft.drawString(msg.text, x + 10, y + bubbleH/2);
        }
    }

    void handleChatTouch() {
        if (!hw->isTouching) return;
        delay(150);

        // Header Back (Torna alla lista contatti)
        if (hw->isTouchInRect(0, 0, 60, 50)) {
            state = MSG_CONTACTS;
            needsRedraw = true;
            return;
        }

        // Input Area (Apri tastiera)
        int inputY = hw->tft.height() - INPUT_H;
        if (hw->touchY > inputY) {
            state = MSG_KEYBOARD;
            needsRedraw = true;
        }
    }

    // --- FUNZIONI DI MESSAGGISTICA ---

    void sendMessage(String text) {
        if (text.length() == 0) return;
        
        // Aggiungi alla chat
        currentChat.push_back({text, true, millis()});
        
        // Aggiorna l'anteprima nella lista contatti
        contacts[selectedContactIdx].lastMsg = "Tu: " + text;
        
        DDO ddo(1);
        ddo.allocatePayload(text.length() + 1);
        memcpy(ddo.getPayloadPtr(), text.c_str(), text.length() + 1);
        system->getNode()->locate(contacts[selectedContactIdx].din, 1);
        system->getNode()->push(contacts[selectedContactIdx].din>>44, &ddo);
    }

    // Helper generico
    void drawHeader(const char* title) {
        hw->tft.fillRect(0, 0, hw->tft.width(), 40, theme->HEADER_BG);
        hw->tft.setTextColor(theme->ACCENT_WARN, theme->HEADER_BG);
        hw->tft.setTextDatum(textdatum_t::middle_center);
        hw->tft.drawString(title, hw->tft.width()/2, 20);
        hw->tft.setTextDatum(textdatum_t::top_left);
        hw->tft.drawString("<", 10, 10);
    }
};