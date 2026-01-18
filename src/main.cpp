#include "os/kernel.hpp"
#include "sys_apps/home.hpp"
#include "sys_apps/settings.hpp"
#include "sys_apps/chat.hpp"


// I don't know why we need this here
extern "C" const char* __dso_handle = 0;
Kernel os;

void setup() {
    // 1. Boot the OS Kernel (Hardware, Theme, SD)
    os.boot();
    os.registerApplication(new HomeApp());
    os.registerApplication(new SettingsApp());
    os.registerApplication(new MessengerApp());

    os.launchApp((u8_t)0); // Home App ID is 0
}

void loop() {
    // 3. Delegate loop to the Kernel (which delegates to the App)
    os.run();
}