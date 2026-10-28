#include "include/led_interface.h"
#include <stddef.h>

void led_init(led_handle_t handle) {
    if (handle && handle->ops && handle->ops->init) {
        handle->is_on = false;
        handle->ops->init(handle);
    }
}

void led_turn_on(led_handle_t handle) {
    if (handle && handle->ops && handle->ops->set_level) {
        handle->ops->set_level(handle, true);
        handle->is_on = true;
    }
}

void led_turn_off(led_handle_t handle) {
    if (handle && handle->ops && handle->ops->set_level) {
        handle->ops->set_level(handle, false);
        handle->is_on = false;
    }
}

void led_toggle(led_handle_t handle) {
    if (handle) {
        if (handle->is_on) {
            led_turn_off(handle);
        } else {
            led_turn_on(handle);
        }
    }
}