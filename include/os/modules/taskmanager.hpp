#pragma once
#include "../interfaces/application_interface.hpp"
#include "os/kernel.hpp"

#define MAX_OPENED_APPS 4
#define MAX_SYS_APPS 16

class TaskManager {
    private:
        Application* openedApp[MAX_OPENED_APPS] = {nullptr}; 
        Application* systemApplications[MAX_SYS_APPS] = {nullptr}; 

    public:

        bool registerApplication(Application* app) {
            // This is used for systen apps only
            for (int i = 0; i < MAX_SYS_APPS; i++) {
                if (systemApplications[i] == nullptr) {
                    systemApplications[i] = app;
                    return true;
                }
            }

            return false;
        }

        Application* openRegisteredApplication(u8_t app_id) {
            // System apps always open in a new slot if available
            for (int i = 0; i < MAX_SYS_APPS; i++) {
                
                // check if it's already opened
                if (systemApplications[i] != nullptr && systemApplications[i]->getAppID() == app_id){

                    if (systemApplications[i]->getPID() == 0){
                        // start the application if not started yet
                        systemApplications[i]->onStart();
                    }

                    return systemApplications[i];
                } 
            }

            Serial.printf("TaskManager: System app with ID %d not registered.\n", app_id);
            return nullptr;
        }

        Application* openApp(Application* &app) {
            for (int i = 0; i < MAX_OPENED_APPS; i++) {
                if (openedApp[i] == nullptr) {
                    openedApp[i] = app;
                    openedApp[i]->setPID(i + 1); // Assign PID
                    return app;
                }
            }

            // if no slot found, we're a circular buffer, close the first one
            printf("TaskManager: Max apps reached, closing oldest app.\n");
            // for now the first one
            openedApp[0]->onExit();
            delete openedApp[0];
            openedApp[0] = app;
            return app; 
        }
};


