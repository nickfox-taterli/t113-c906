/*
 * arch/riscv/trap.c - RISC-V exception and trap handling
 *
 * This file contains exception handling routines for the C906 RISC-V processor.
 */

#include <stdint.h>
#include <stdio.h>
#include "excep.h"
#include "csr.h"
#include "softirq.h"
#include "plic.h"

/**
 * riscv_trap_handler - Default exception/trap handler
 * @mcause: Machine Cause Register value
 * @mepc:   Machine Exception Program Counter
 * @mtval:  Machine Trap Value Register
 * @regs:   Pointer to interrupt register context
 *
 * Returns: Updated program counter value (normally unchanged)
 *
 * This is the default handler for unhandled exceptions. It prints
 * diagnostic information and halts the system.
 */
unsigned long riscv_trap_handler(unsigned long mcause, unsigned long mepc,
                                 unsigned long mtval, irq_regs_t *regs)
{
    (void)regs;

    printf("Unhandled exception: mcause=0x%lx, mepc=0x%lx, mtval=0x%lx\n",
           mcause, mepc, mtval);

    /* Halt on unhandled exception */
    while (1) {
        /* Infinite loop */
    }

    return mepc;
}

/**
 * riscv_cpu_handle_exception - Exception handler wrapper
 *
 * This function is called from portASM.S and wraps riscv_trap_handler.
 * It's part of the FreeRTOS port interface.
 */
void riscv_cpu_handle_exception(void)
{
    /* This function is called from assembly context
     * The actual exception handling is done by riscv_trap_handler
     * which is called from head_s.S */
}

/**
 * C906_Default_IRQHandler - Default interrupt handler for C906
 *
 * This is the external interrupt handler called from portASM.S.
 * It delegates to the PLIC interrupt handler.
 */
void C906_Default_IRQHandler(void)
{
    /* Provided by FreeRTOS port_misc.c */
    extern void enter_interrupt_handler(void);
    extern void exit_interrupt_handler(void);

    enter_interrupt_handler();
    /* PLIC handler will be called from here */
    plic_handle_irq(NULL);
    exit_interrupt_handler();
}

/**
 * c906_default_irq_handler - Default IRQ handler for C906
 *
 * This is the default interrupt handler called for external interrupts.
 * It delegates to the PLIC interrupt handler.
 */
void c906_default_irq_handler(void)
{
    C906_Default_IRQHandler();
}
