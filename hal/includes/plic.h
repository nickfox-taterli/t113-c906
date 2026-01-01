#ifndef PLIC_H
#define PLIC_H

#include <stdint.h>

/**
 * plic_init - Initialize PLIC interrupt controller
 */
void plic_init(void);

/**
 * plic_irq_enable - Enable a PLIC interrupt
 * @hwirq: Hardware interrupt number
 */
void plic_irq_enable(unsigned int hwirq);

/**
 * plic_irq_disable - Disable a PLIC interrupt
 * @hwirq: Hardware interrupt number
 */
void plic_irq_disable(unsigned int hwirq);

/**
 * plic_irq_eoi - End of interrupt for PLIC
 * @hwirq: Hardware interrupt number
 */
void plic_irq_eoi(unsigned int hwirq);

/* Forward declaration */
struct irq_regs;

/**
 * plic_handle_irq - PLIC interrupt handler
 * @regs: Pointer to interrupt register context
 */
void plic_handle_irq(struct irq_regs *regs);

/* Generic IRQ handler - provided by architecture layer */
extern int generic_handle_irq(unsigned int irq);

#endif /* PLIC_H */
