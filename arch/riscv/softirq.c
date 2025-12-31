/*
 * arch/riscv/softirq.c - RISC-V software interrupt handling for C906
 *
 * This file contains software interrupt (soft IRQ) routines for inter-processor
 * communication and context switching in FreeRTOS.
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "csr.h"
#include "clic.h"

/**
 * riscv_trigger_soft_interrupt - Trigger software interrupt
 *
 * This function writes to the Machine Software Interrupt Pending (MSIP)
 * register to trigger a software interrupt. This is used to force a context
 * switch in FreeRTOS.
 */
void riscv_trigger_soft_interrupt(void)
{
    volatile uint32_t *msip_reg = (volatile uint32_t *)clic_int_control_addr_base;

    if (msip_reg) {
        *msip_reg = 1;
        /* Memory barriers to ensure write completes */
        __asm__ volatile("fence iorw,iorw");
        __asm__ volatile("sync");
    }
}

/**
 * riscv_soft_irq_handler - Software interrupt handler (non-vector mode)
 *
 * This function handles software interrupts in non-vector mode. It clears
 * the MSIP register and performs a context switch.
 */
void riscv_soft_irq_handler(void)
{
    volatile uint32_t *msip_reg = (volatile uint32_t *)clic_int_control_addr_base;

    if (msip_reg) {
        /* Clear the software interrupt */
        *msip_reg = 0;
        /* Memory barriers to ensure write completes */
        __asm__ volatile("fence iorw,iorw");
        __asm__ volatile("sync");
    }

    /* Trigger FreeRTOS context switch */
    vTaskSwitchContext();
}

/**
 * rv_soft_irq_handler_non_vec_mode - Software interrupt handler (alias)
 *
 * This is an alias for riscv_soft_irq_handler used by the FreeRTOS
 * portASM.S assembly file.
 */
void rv_soft_irq_handler_non_vec_mode(void)
{
    riscv_soft_irq_handler();
}

/**
 * rv_trigger_soft_interrupt - Trigger software interrupt (alias)
 *
 * This is an alias for riscv_trigger_soft_interrupt used by the FreeRTOS
 * tasks.c source file.
 */
void rv_trigger_soft_interrupt(void)
{
    riscv_trigger_soft_interrupt();
}
