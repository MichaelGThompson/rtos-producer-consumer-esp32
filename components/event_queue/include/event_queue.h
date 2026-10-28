#pragma once

#include <stdint.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t  event_id;  // Identifies which button was pressed
    uint64_t timestamp;  // System tick or millisecond timer value
} ButtonEvent_t;

#define QUEUE_SIZE 10

typedef struct {
    ButtonEvent_t data[QUEUE_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} ButtonQueue_t;

void Queue_Init(ButtonQueue_t *q);
bool Queue_IsFull(ButtonQueue_t *q);
bool Queue_IsEmpty(ButtonQueue_t *q);
bool Queue_Enqueue(ButtonQueue_t *q, uint8_t button_id); //, uint64_t timestamp);
bool Queue_Dequeue(ButtonQueue_t *q, ButtonEvent_t *event_out);

#ifdef __cplusplus
}
#endif
