#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "event_queue.h"
#include "fsm.h"
#include "led_interface.h"

#define LED_2           GPIO_NUM_8
#define LED_1           GPIO_NUM_1
#define BUTTON_GPIO     GPIO_NUM_0

static const char *TAG = "event-queue-demo";

// Forward declaration of our ESP32 factory function
led_handle_t led_esp32_gpio_create(gpio_num_t pin_num, bool active_low);

ButtonQueue_t queue;
uint8_t event_id = 0;

static void IRAM_ATTR gpio0_isr(void *arg)
{
    (void)arg;
    ESP_DRAM_LOGI(DRAM_STR("event-queue-demo"), "--- external interrupt enqueued event ---");
    Queue_Enqueue(&queue, event_id++);
}

static void gpio0_interrupt_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = 1ULL << GPIO_NUM_0,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(GPIO_NUM_0, gpio0_isr, NULL));
    ESP_LOGI(TAG, "--- Button is ready to trigger interrupt ---");
}

void app_main(void) {
    vTaskDelay(500 / portTICK_PERIOD_MS);

    Queue_Init(&queue);
    ButtonEvent_t event_out = { 0 };

    // initialize button interrupt on pin 0
    gpio0_interrupt_init();

    // initialize leds
    led_handle_t system_leds[] = {
        led_esp32_gpio_create(GPIO_NUM_1, false),  // Error Red LED (Active High)
        led_esp32_gpio_create(GPIO_NUM_8, false)    // Network Green LED (Active Low)
    };
    
    size_t num_leds = sizeof(system_leds) / sizeof(system_leds[0]);

    // Initialize all LEDs blindly using the interface
    for (size_t i = 0; i < num_leds; i++) {
        if (system_leds[i] != NULL) {
            led_init(system_leds[i]);
        }
    }

    ESP_LOGI(TAG, "--- LEDs ready ---");

    vTaskDelay(500 / portTICK_PERIOD_MS);

    led_turn_off(system_leds[0]);
    led_turn_off(system_leds[1]);

    while (1) {

        led_toggle(system_leds[0]);
        led_toggle(system_leds[1]);

        if (!Queue_IsEmpty(&queue)) {
            if (Queue_Dequeue(&queue, &event_out)) {
                ESP_LOGI(TAG, "--- dequeued event_id: %d ---", event_out.event_id);

                fsm_state_t next = fsm_table[current_state].next_state;
                current_state = next;
                if (fsm_table[current_state].state_action != NULL) {
                    fsm_table[current_state].state_action();
                }
            }
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS); // Wait 1 second
    }
}
