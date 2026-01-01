/*
 * hal/includes/irq.h - HAL interrupt control interface
 */

#ifndef HAL_IRQ_H
#define HAL_IRQ_H

#include <stdint.h>

/* HAL interrupt control functions - provided by architecture layer */
extern unsigned long xport_interrupt_disable(void);
extern void xport_interrupt_enable(unsigned long flags);

/* Generic IRQ registration */
typedef void (*irq_handler_t)(void *param);

/*
 * Register a handler for a given IRQ number and enable it in the PLIC.
 * Returns 0 on success.
 */
int irq_request(unsigned int irq_no, irq_handler_t handler, void *param, const char *name);

/* Disable the IRQ in the PLIC and remove the handler */
int irq_free(unsigned int irq_no);

/* HAL interrupt control macros */
#define hal_interrupt_disable_irqsave()   xport_interrupt_disable()
#define hal_interrupt_enable_irqrestore(flags)  xport_interrupt_enable(flags)

#endif /* HAL_IRQ_H */
