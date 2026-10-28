#include <stdint.h>
#include <stdbool.h>

#include "include/event_queue.h"

void Queue_Init(ButtonQueue_t *q) {
    q->head = 0;
    q->tail = 0;
}

bool Queue_IsFull(ButtonQueue_t *q) {
    return ((q->head + 1) % QUEUE_SIZE) == q->tail;
}

bool Queue_IsEmpty(ButtonQueue_t *q) {
    return q->head == q->tail;
}

// 6. Enqueue: Add button press (Typically called inside an ISR)
bool Queue_Enqueue(ButtonQueue_t *q, uint8_t button_id) {  //, uint32_t timestamp) {
    if (Queue_IsFull(q)) {
        return false; // Queue overflow! Handle appropriately (e.g., overwrite or drop)
    }
    
    q->data[q->head].event_id = button_id;
    //q->data[q->head].timestamp = timestamp;
    
    q->head = (q->head + 1) % QUEUE_SIZE;
    return true;
}

// 7. Dequeue: Remove and process event (Typically called in main loop)
bool Queue_Dequeue(ButtonQueue_t *q, ButtonEvent_t *event_out) {
    if (Queue_IsEmpty(q)) {
        return false; // Nothing to process
    }
    
    *event_out = q->data[q->tail];
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    return true;
}
