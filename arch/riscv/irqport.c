/*
 * arch/riscv/irqport.c - FreeRTOS interrupt port layer
 *
 * This file provides the FreeRTOS port layer interrupt control functions
 * for the C906 RISC-V processor.
 */

#include "irqflags.h"

/**
 * xport_interrupt_disable - Disable interrupts (FreeRTOS port)
 *
 * Returns: Current interrupt state flags
 *
 * This function saves the current interrupt state and disables interrupts.
 * It is called by FreeRTOS to enter critical sections.
 */
unsigned long xport_interrupt_disable(void)
{
    return arch_local_irq_save();
}

/**
 * xport_interrupt_enable - Enable interrupts (FreeRTOS port)
 * @flags: Interrupt state flags to restore
 *
 * This function restores the interrupt state to the value saved by
 * xport_interrupt_disable(). It is called by FreeRTOS to exit
 * critical sections.
 */
void xport_interrupt_enable(unsigned long flags)
{
    arch_local_irq_restore(flags);
}
