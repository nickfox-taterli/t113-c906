#include "cache.h"
#include "ccu.h"
#include "uart.h"
#include "plic.h"
#include "clic.h"
#include "timer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "openamp/platform/platform.h"
#include <stdio.h>

/* Global UART configuration */
static uart_config_t uart_cfg;
static struct openamp_platform openamp_ctx;

/**
 * main_task - Main FreeRTOS task
 * @pvParameters: Task parameters (unused)
 *
 * This task prints a message every 500ms with the current cycle count.
 */
static void main_task(void *pvParameters)
{
    (void)pvParameters;
    int ret;

    printf("OpenAMP: init begin\n");
    ret = platform_rproc_init(&openamp_ctx);
    if (ret) {
        printf("OpenAMP init failed (%d)\n", ret);
        goto halt;
    }

    printf("OpenAMP init ok, echo ready.\n");

    /* Task main loop */
    while (1) {
        platform_poll(&openamp_ctx);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

halt:
    printf("OpenAMP task halted.\n");
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void low_level_init(void)
{
    /* Initialize cache */
    CacheInitialize();
    InstructionCacheEnable();
    DataCacheEnable();

    /* Enable UART2 clock */
    CCU_EnableClock(BUS_UART2);
    CCU_Reset(RST_UART2);

    /* Configure UART2 */
    uart_cfg.base_addr = SUNXI_UART2_BASE;
    uart_cfg.uart_clk = 24000000;
    uart_cfg.baudrate = 115200;
    uart_cfg.data_bits = 8;
    uart_cfg.stop_bits = 1;
    uart_cfg.parity = 0;

    uart_init(&uart_cfg);

    /* Initialize interrupt controllers and timer */
    plic_init();
    clic_driver_init();
    riscv_system_tick_init();
}

void start_kernel(void)
{
    BaseType_t xReturned;

    /* Initialize hardware peripherals */
    low_level_init();

    printf("Starting FreeRTOS on C906...\n");

    /* Create main task */
    xReturned = xTaskCreate(
        main_task,                /* Task function */
        "MainTask",               /* Task name */
        configMINIMAL_STACK_SIZE * 8, /* Stack size */
        NULL,                     /* Parameters */
        tskIDLE_PRIORITY + 1,     /* Priority */
        NULL);                    /* Handle */

    if (xReturned == pdPASS) {
        printf("Main task created successfully!\n");
        /* Start FreeRTOS scheduler */
        vTaskStartScheduler();

        /* Should not reach here */
        printf("Scheduler returned unexpectedly!\n");
    } else {
        printf("Failed to create main task!\n");
    }

    /* System halt on failure */
    printf("System halted!\n");
    while (1) {
        /* Infinite loop */
    }
}
