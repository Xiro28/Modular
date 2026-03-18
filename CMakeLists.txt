cmake_minimum_required(VERSION 3.16.0)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Modular)
# HACK: Forza l'abilitazione del DMA in PSRAM per bypassare il bug del componente esp_lvgl_port
add_compile_options("-DCONFIG_ESP_LCD_COMMON_MEM_ALLOC_IN_PSRAM=1")