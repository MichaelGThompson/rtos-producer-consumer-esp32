/*
 * Unity tests for the pc_queue component.
 * pioarduino builds this file instead of main/main.c when you run `pio test`.
 * Assertions stay on the test task. Worker tasks only record failures.
 */

#include "unity.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "pc_queue.h"

#define TASK_ITEM_COUNT 8
#define TASK_QUEUE_CAPACITY 2

typedef struct {
    pc_queue_t *queue;
    SemaphoreHandle_t done;
    uint32_t failures;
} task_sync_t;

void setUp(void)
{
}

void tearDown(void)
{
}

static void producer_task(void *arg)
{
    task_sync_t *sync = arg;

    for (uint32_t i = 0; i < TASK_ITEM_COUNT; i++) {
        pc_item_t item = {
            .sequence = i,
            .value = (int32_t)(1000 + i),
        };
        if (pc_queue_produce(sync->queue, &item, 1000) != ESP_OK) {
            sync->failures++;
        }
    }

    xSemaphoreGive(sync->done);
    vTaskDelete(NULL);
}

static void consumer_task(void *arg)
{
    task_sync_t *sync = arg;

    for (uint32_t i = 0; i < TASK_ITEM_COUNT; i++) {
        pc_item_t item = {0};
        if (pc_queue_consume(sync->queue, &item, 1000) != ESP_OK ||
            item.sequence != i ||
            item.value != (int32_t)(1000 + i)) {
            sync->failures++;
        }
    }

    xSemaphoreGive(sync->done);
    vTaskDelete(NULL);
}

static void test_create_rejects_zero_capacity(void)
{
    TEST_ASSERT_NULL(pc_queue_create(0));
}

static void test_destroy_accepts_null(void)
{
    pc_queue_destroy(NULL);
}

static void test_capacity_and_empty_count(void)
{
    pc_queue_t *queue = pc_queue_create(3);

    TEST_ASSERT_NOT_NULL(queue);
    TEST_ASSERT_EQUAL_UINT32(3, pc_queue_capacity(queue));
    TEST_ASSERT_EQUAL_UINT32(0, pc_queue_count(queue));
    TEST_ASSERT_EQUAL_UINT32(0, pc_queue_count(NULL));
    TEST_ASSERT_EQUAL_UINT32(0, pc_queue_capacity(NULL));

    pc_queue_destroy(queue);
}

static void test_produce_and_consume_preserve_fifo_order(void)
{
    pc_queue_t *queue = pc_queue_create(4);
    TEST_ASSERT_NOT_NULL(queue);

    for (uint32_t i = 0; i < 3; i++) {
        pc_item_t item = {
            .sequence = i,
            .value = (int32_t)(i * 10),
        };
        TEST_ASSERT_EQUAL(ESP_OK, pc_queue_produce(queue, &item, 0));
    }
    TEST_ASSERT_EQUAL_UINT32(3, pc_queue_count(queue));

    for (uint32_t i = 0; i < 3; i++) {
        pc_item_t item = {0};
        TEST_ASSERT_EQUAL(ESP_OK, pc_queue_consume(queue, &item, 0));
        TEST_ASSERT_EQUAL_UINT32(i, item.sequence);
        TEST_ASSERT_EQUAL_INT32((int32_t)(i * 10), item.value);
    }
    TEST_ASSERT_EQUAL_UINT32(0, pc_queue_count(queue));

    pc_queue_destroy(queue);
}

static void test_consume_on_empty_times_out(void)
{
    pc_queue_t *queue = pc_queue_create(1);
    pc_item_t item = {0};

    TEST_ASSERT_NOT_NULL(queue);
    TEST_ASSERT_EQUAL(ESP_ERR_TIMEOUT, pc_queue_consume(queue, &item, 20));

    pc_queue_destroy(queue);
}

static void test_produce_on_full_times_out(void)
{
    pc_queue_t *queue = pc_queue_create(1);
    pc_item_t item = {.sequence = 1, .value = 2};
    pc_item_t extra = {.sequence = 9, .value = 9};

    TEST_ASSERT_NOT_NULL(queue);
    TEST_ASSERT_EQUAL(ESP_OK, pc_queue_produce(queue, &item, 0));
    TEST_ASSERT_EQUAL(ESP_ERR_TIMEOUT, pc_queue_produce(queue, &extra, 20));
    TEST_ASSERT_EQUAL_UINT32(1, pc_queue_count(queue));

    pc_queue_destroy(queue);
}

static void test_null_arguments_are_rejected(void)
{
    pc_queue_t *queue = pc_queue_create(1);
    pc_item_t item = {0};

    TEST_ASSERT_NOT_NULL(queue);
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, pc_queue_produce(NULL, &item, 0));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, pc_queue_produce(queue, NULL, 0));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, pc_queue_consume(NULL, &item, 0));
    TEST_ASSERT_EQUAL(ESP_ERR_INVALID_ARG, pc_queue_consume(queue, NULL, 0));

    pc_queue_destroy(queue);
}

static void test_tasks_exchange_items_in_order(void)
{
    task_sync_t producer = {
        .queue = pc_queue_create(TASK_QUEUE_CAPACITY),
        .done = xSemaphoreCreateBinary(),
        .failures = 0,
    };
    task_sync_t consumer = {
        .queue = producer.queue,
        .done = xSemaphoreCreateBinary(),
        .failures = 0,
    };

    TEST_ASSERT_NOT_NULL(producer.queue);
    TEST_ASSERT_NOT_NULL(producer.done);
    TEST_ASSERT_NOT_NULL(consumer.done);

    TEST_ASSERT_EQUAL(pdPASS, xTaskCreate(
        producer_task, "pc-prod", 4096, &producer, 5, NULL));
    TEST_ASSERT_EQUAL(pdPASS, xTaskCreate(
        consumer_task, "pc-cons", 4096, &consumer, 5, NULL));

    TEST_ASSERT_EQUAL(pdTRUE, xSemaphoreTake(producer.done, pdMS_TO_TICKS(5000)));
    TEST_ASSERT_EQUAL(pdTRUE, xSemaphoreTake(consumer.done, pdMS_TO_TICKS(5000)));
    TEST_ASSERT_EQUAL_UINT32(0, producer.failures);
    TEST_ASSERT_EQUAL_UINT32(0, consumer.failures);
    TEST_ASSERT_EQUAL_UINT32(0, pc_queue_count(producer.queue));

    vSemaphoreDelete(producer.done);
    vSemaphoreDelete(consumer.done);
    pc_queue_destroy(producer.queue);
}

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(500));

    UNITY_BEGIN();
    RUN_TEST(test_create_rejects_zero_capacity);
    RUN_TEST(test_destroy_accepts_null);
    RUN_TEST(test_capacity_and_empty_count);
    RUN_TEST(test_produce_and_consume_preserve_fifo_order);
    RUN_TEST(test_consume_on_empty_times_out);
    RUN_TEST(test_produce_on_full_times_out);
    RUN_TEST(test_null_arguments_are_rejected);
    RUN_TEST(test_tasks_exchange_items_in_order);
    UNITY_END();
}
