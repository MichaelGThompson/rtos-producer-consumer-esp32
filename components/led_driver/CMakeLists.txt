idf_component_register(
    SRCS "led_interface.c" "led_hw_esp32.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio esp_common
)
