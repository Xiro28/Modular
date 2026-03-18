idf_component_register(
    SRCS "main.cpp" "kernel.cpp" "daas_interfaces.cpp"
    INCLUDE_DIRS "../include"
    # esp_wifi rimosso realmente. Aggiunto nvs_flash che serve a netif
    REQUIRES lvgl lwip esp_netif esp_event nvs_flash esp_hosted
)

# Se non usi più LovyanGFX sul P4 (visto che usi il BSP), non serve dichiararlo.

# Configure libdaas.a as an imported library
add_library(daas STATIC IMPORTED)
set_target_properties(daas PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libdaas.a")

# Tell CMake daas depends ONLY on lwip
set_property(TARGET daas APPEND PROPERTY INTERFACE_LINK_LIBRARIES idf::lwip)

target_link_libraries(${COMPONENT_LIB} PUBLIC daas)