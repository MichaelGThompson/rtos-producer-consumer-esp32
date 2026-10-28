#include "include/fsm.h"

void display_all_off(void) {
    ESP_LOGI(FSM_TAG, "[Action] Clearing outputs -> ALL OFF\n");
    // gpio_set_level(RED_LED_GPIO, 0);
    // gpio_set_level(GREEN_LED_GPIO, 0);
}

void display_red_on(void) {
    ESP_LOGI(FSM_TAG, "[Action] Modifying pins -> RED ONLY\n");
    // gpio_set_level(RED_LED_GPIO, 1);
    // gpio_set_level(GREEN_LED_GPIO, 0);
}

void display_green_on(void) {
    ESP_LOGI(FSM_TAG, "[Action] Modifying pins -> GREEN ONLY\n");
    // gpio_set_level(RED_LED_GPIO, 0);
    // gpio_set_level(GREEN_LED_GPIO, 1);
}

void display_all_on(void) {
    ESP_LOGI(FSM_TAG, "[Action] Driving outputs -> ALL ON\n");
//     gpio_set_level(RED_LED_GPIO, 1);
//     gpio_set_level(GREEN_LED_GPIO, 1);
}