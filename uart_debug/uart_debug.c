#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"



#define DEBUG_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define DEBUG_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define DEBUG_TEST_RTS (UART_PIN_NO_CHANGE)
#define DEBUG_TEST_CTS (UART_PIN_NO_CHANGE)

#define DEBUG_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define DEBUG_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define DEBUG_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

static const char *TAG = "UART TEST";

#define BUF_SIZE (1024)

static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    const uart_port_t uart_num = UART_NUM_2;
    uart_config_t uart_config = {
        .baud_rate = DEBUG_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    QueueHandle_t uart_queue;

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart_queue, 0));
   
    // Configure a temporary buffer for the incoming data
    uint8_t *data[128];

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(DEBUG_UART_PORT_NUM, data, (128 - 1), 10);
        // Write data back to the UART
        uart_write_bytes(DEBUG_UART_PORT_NUM, (const char *) data, len);
        if (len) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recv str: %s", (char *) data);
        }
    }
}

void app_main(void)
{
    xTaskCreate(echo_task, "uart_echo_task", DEBUG_TASK_STACK_SIZE, NULL, 10, NULL);
}