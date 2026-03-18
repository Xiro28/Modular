#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

static inline uint64_t millis() {
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}

static inline void delay(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

#if CYD
    #include "cyd/cyd.hpp"
#else
    #include "p4/esp32p4.hpp"
#endif

