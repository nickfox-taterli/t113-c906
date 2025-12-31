/*
 * arch/includes/arch/riscv/softirq.h - RISC-V software interrupt interface
 */

#ifndef _ARCH_RISCV_SOFTIRQ_H
#define _ARCH_RISCV_SOFTIRQ_H

/**
 * riscv_trigger_soft_interrupt - Trigger software interrupt
 */
void riscv_trigger_soft_interrupt(void);

/**
 * riscv_soft_irq_handler - Software interrupt handler
 */
void riscv_soft_irq_handler(void);

/**
 * rv_soft_irq_handler_non_vec_mode - Software interrupt handler (alias)
 *
 * This is an alias for riscv_soft_irq_handler used by the FreeRTOS
 * portASM.S assembly file.
 */
void rv_soft_irq_handler_non_vec_mode(void);

/**
 * rv_trigger_soft_interrupt - Trigger software interrupt (alias)
 *
 * This is an alias for riscv_trigger_soft_interrupt used by the FreeRTOS
 * tasks.c source file.
 */
void rv_trigger_soft_interrupt(void);

#endif /* _ARCH_RISCV_SOFTIRQ_H */
