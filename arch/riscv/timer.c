/*
 * arch/riscv/timer.c - RISC-V timer handling for C906
 *
 * This file contains timer initialization and interrupt handling routines
 * for the C906 CLINT/CLIC timer.
 */

#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "csr.h"
#include "rv_io.h"
#include "clic.h"

/* CLINT/CLIC timer configuration is set by clic_driver_init(). */

/*
 * FreeRTOS timer variables (provided by port layer)
 * These are referenced in timer interrupt context
 */
extern volatile uint64_t *pullMachineTimerCompareRegister;
extern uint64_t ullNextTime;
extern const size_t uxTimerIncrementsForOneTick;

/* System tick interval based on CPU clock and tick rate */
#define TICK_DELTA ((configCPU_CLOCK_HZ) / (configTICK_RATE_HZ))

/**
 * riscv_get_cycles - Read 64-bit cycle counter
 *
 * Returns: Current 64-bit cycle count from TIME CSR
 */
static inline unsigned long riscv_get_cycles(void)
{
    /* C906 TIME register is the CLINT counter */
    return csr_read(CSR_TIME);
}

/**
 * riscv_timer_interrupt - Machine timer interrupt handler
 *
 * This function is called when the machine timer interrupt fires.
 * It reprograms the timer for the next tick and re-enables the interrupt.
 */
void riscv_timer_interrupt(void)
{
    unsigned int mask = ~0U;
    unsigned long next_compare;
    unsigned long addr = clic_int_control_addr_comp;

    /* Calculate next compare value */
    next_compare = riscv_get_cycles() + TICK_DELTA;

    /* Disable timer interrupt temporarily */
    csr_clear(mie, MIE_MTIE);

    /* Write new compare value (64-bit split into two 32-bit writes) */
    writel_relaxed(next_compare & mask, (void *)addr);
    writel_relaxed(next_compare >> 32, (void *)(addr) + 0x04);

    /* Re-enable timer interrupt */
    csr_set(mie, MIE_MTIE);
}

/**
 * riscv_system_tick_init - Initialize system tick timer
 *
 * This function initializes the machine timer for system ticks.
 * It sets up the first timer interrupt and configures the timer registers.
 */
void riscv_system_tick_init(void)
{
    unsigned int mask = ~0U;
    unsigned long next_compare;
    unsigned long addr = clic_int_control_addr_comp;

    /* Calculate first compare value */
    next_compare = riscv_get_cycles() + TICK_DELTA;

    /* Pre-populate FreeRTOS port layer pointers to avoid early NULL access */
    if (addr != 0) {
        pullMachineTimerCompareRegister = (volatile uint64_t *)addr;
        ullNextTime = next_compare + TICK_DELTA;
    }

    /* Enable machine timer interrupt */
    csr_set(mie, MIE_MTIE);

    /* Write initial compare value */
    writel_relaxed(next_compare & mask, (void *)addr);
    writel_relaxed(next_compare >> 32, (void *)(addr) + 0x04);
}

/**
 * riscv_timer_port_setup - FreeRTOS port timer setup
 *
 * This function overrides the weak vPortSetupTimerInterrupt from the
 * FreeRTOS port layer. It configures the timer using the actual CLIC/CLINT
 * address instead of NULL, preventing crashes in portASM.S.
 */
void riscv_timer_port_setup(void)
{
    uint64_t first_compare;
    unsigned long addr = clic_int_control_addr_comp;

    if (addr == 0) {
        printf("riscv_timer_port_setup: invalid timer base\n");
        return;
    }

    /* Set up the timer register pointer for FreeRTOS */
    pullMachineTimerCompareRegister = (volatile uint64_t *)addr;

    /* Schedule first tick */
    first_compare = riscv_get_cycles() + uxTimerIncrementsForOneTick;
    writel_relaxed((uint32_t)(first_compare & 0xffffffffU), (void *)addr);
    writel_relaxed((uint32_t)(first_compare >> 32), (void *)(addr + 0x04));

    /* Pre-calculate next tick time for use in portASM.S */
    ullNextTime = first_compare + uxTimerIncrementsForOneTick;
}
