#pragma once

#include <stdint.h>
#include "esp_log.h"

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char *FSM_TAG = "fsm for LEDs";

// fsm for displaying sequence of 2 leds
typedef enum {
    STATE_ALL_OFF,
    STATE_RED_ON,
    STATE_GREEN_ON,
    STATE_ALL_ON
} fsm_state_t;

void display_all_off(void);
void display_red_on(void);
void display_green_on(void);
void display_all_on(void);

typedef struct {
    fsm_state_t next_state;  
    void (*state_action)(void); 
} fsm_transition_t;

static const fsm_transition_t fsm_table[STATE_ALL_ON + 1] = {
    [STATE_ALL_OFF]  = { .next_state = STATE_RED_ON,    .state_action = display_all_off   },
    [STATE_RED_ON]   = { .next_state = STATE_GREEN_ON,  .state_action = display_red_on    },
    [STATE_GREEN_ON] = { .next_state = STATE_ALL_ON,    .state_action = display_green_on  },
    [STATE_ALL_ON]   = { .next_state = STATE_ALL_OFF,   .state_action = display_all_on    }
};

// Global state tracker
static fsm_state_t current_state = STATE_ALL_OFF;

#ifdef __cplusplus
}
#endif
