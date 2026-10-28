#include "include/led_interface.h"
#include "driver/gpio.h"

// Concrete object defining an ESP32 GPIO-based LED
typedef struct {
    led_interface_t base;   // Must be the first element for safe pointer casting
    gpio_num_t pin_num;     // ESP32 specific type
    bool active_low;        // Hardware wiring type inversion control
} esp32_gpio_led_t;

// Implementation of the interface init wrapper
static void esp32_gpio_init_impl(led_handle_t handle) {
    esp32_gpio_led_t* esp_led = (esp32_gpio_led_t*)handle;
    
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << esp_led->pin_num),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&io_conf);
}

// Implementation of the interface set_level wrapper
static void esp32_gpio_set_level_impl(led_handle_t handle, bool level) {
    esp32_gpio_led_t* esp_led = (esp32_gpio_led_t*)handle;
    
    // Invert the physical signal if the board layout is active-low
    bool physical_state = esp_led->active_low ? !level : level;
    gpio_set_level(esp_led->pin_num, physical_state);
}

// The shared operations table for any ESP32 Standard GPIO LED
static const led_ops_t esp32_gpio_ops = {
    .init = esp32_gpio_init_impl,
    .set_level = esp32_gpio_set_level_impl
};

// Public constructor to instantiate a new hardware instance safely
led_handle_t led_esp32_gpio_create(gpio_num_t pin_num, bool active_low) {
    // Allocation can be static or dynamic. Using static allocations here via a pool, 
    // or you can pass static structs from main. Let's provide a definition to build safely:
    static esp32_gpio_led_t led_pool[10]; 
    static int pool_index = 0;
    
    if (pool_index >= 10) return NULL;
    
    esp32_gpio_led_t* new_led = &led_pool[pool_index++];
    new_led->base.ops = &esp32_gpio_ops;
    new_led->pin_num = pin_num;
    new_led->active_low = active_low;
    
    return (led_handle_t)new_led;
}