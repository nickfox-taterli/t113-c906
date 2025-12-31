/*
 * hal/includes/irq.h - HAL interrupt control interface
 */

#ifndef HAL_IRQ_H
#define HAL_IRQ_H

#include <stdint.h>

/* HAL interrupt control functions - provided by architecture layer */
extern unsigned long xport_interrupt_disable(void);
extern void xport_interrupt_enable(unsigned long flags);

/* HAL interrupt control macros */
#define hal_interrupt_disable_irqsave()   xport_interrupt_disable()
#define hal_interrupt_enable_irqrestore(flags)  xport_interrupt_enable(flags)

#endif /* HAL_IRQ_H */
