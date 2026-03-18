#include "os/kernel.hpp"
#include "sys_apps/boot.hpp"
#include "sys_apps/home.hpp"
//#include "sys_apps/settings.hpp"
#include "sys_apps/terminal.hpp"
#include "sys_apps/chat.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "bsp/esp-bsp.h"

#include "slave_firmware.hpp"

#include "esp_hosted.h"

Kernel* os = nullptr;


extern "C" void app_main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("\n[DEBUG] Entrato in app_main\n");

    printf("Update slave firmware...\n");
    esp_hosted_coprocessor_fwver_t old_firmware_version;
    esp_hosted_get_coprocessor_fwversion(&old_firmware_version);
    printf("Current slave firmware version: %d.%d.%d\n", old_firmware_version.major, old_firmware_version.minor, old_firmware_version.patch);

    esp_hosted_slave_ota_begin();
    esp_hosted_slave_ota_write((const uint8_t*)slave_firmware, sizeof(slave_firmware));
    esp_hosted_slave_ota_end();
    printf("Slave firmware update completed.\n");

    try {
        printf("[DEBUG] Allocazione Kernel...\n");
        os = new Kernel();
        printf("[DEBUG] Allocazione completata\n");

        printf("[DEBUG] Avvio boot hardware...\n");
        os->boot(); 
        printf("[DEBUG] Boot hardware completato\n");

        bsp_display_lock(0);
        printf("[DEBUG] Creazione Applicazioni...\n");
        os->registerApplication(new BootApp());
        os->registerApplication(new HomeApp());
        //xos->registerApplication(new SettingsApp());
        os->registerApplication(new MessengerApp());
        os->registerApplication(new TerminalApp());
        
        os->launchApp(99);
        bsp_display_unlock();
        
        printf("[DEBUG] Modular OS Pronto!\n");
    } catch (const std::exception& e) {
        printf("[ERRORE FATALE] Eccezione C++: %s\n", e.what());
    }

    while (1) {
        os->run();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}