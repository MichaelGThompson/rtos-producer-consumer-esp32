#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration of the interface structure (keeps implementation hidden)
typedef struct led_interface_s led_interface_t;

// Opaque handle used by the application layer
typedef led_interface_t* led_handle_t;

// Definition of the operations table (function pointers)
typedef struct {
    void (*init)(led_handle_t handle);
    void (*set_level)(led_handle_t handle, bool level);
} led_ops_t;

// The base structure that bridges logic and hardware
struct led_interface_s {
    const led_ops_t* ops;  // Table of hardware-specific functions
    bool is_on;            // Monitored state logic
};

// Hardware-agnostic Public APIs
void led_init(led_handle_t handle);
void led_turn_on(led_handle_t handle);
void led_turn_off(led_handle_t handle);
void led_toggle(led_handle_t handle);

#ifdef __cplusplus
}
#endif
