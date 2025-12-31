/*
 * arch/includes/arch/riscv/trap.h - RISC-V exception handling interface
 */

#ifndef _ARCH_RISCV_TRAP_H
#define _ARCH_RISCV_TRAP_H

#include "excep.h"

/**
 * riscv_trap_handler - Default exception/trap handler
 * @mcause: Machine Cause Register value
 * @mepc:   Machine Exception Program Counter
 * @mtval:  Machine Trap Value Register
 * @regs:   Pointer to interrupt register context
 *
 * Returns: Updated program counter value
 */
unsigned long riscv_trap_handler(unsigned long mcause, unsigned long mepc,
                                  unsigned long mtval, irq_regs_t *regs);

/**
 * c906_default_irq_handler - Default IRQ handler for C906
 */
void c906_default_irq_handler(void);

#endif /* _ARCH_RISCV_TRAP_H */
