/*
 * arch/riscv/irq.c - RISC-V interrupt management
 *
 * This file contains architecture-level interrupt management functions
 * for the C906 RISC-V processor.
 */

#include <stdint.h>
#include <stdbool.h>
#include "csr.h"
#include "irqflags.h"
#include "irqs.h"
#include "plic.h"

/* 
 * Generic IRQ handler - placeholder for device-specific handlers
 * This should be overridden by actual device interrupt handlers
 */
void generic_handle_irq(unsigned int irq)
{
    /* Placeholder - device-specific handlers should implement this */
}

/* 
 * Interrupt tracking bitmap for PLIC interrupts
 * 32-bit words, each bit represents one IRQ line
 */
uint32_t plic_irq_rec[(NR_IRQS + 1 + 32) >> 5] = {0};

/**
 * irq_enable - Enable a specific interrupt
 * @irq_no: Interrupt number to enable
 *
 * Returns: 0 on success
 *
 * This function enables the specified interrupt line and updates
 * the interrupt tracking bitmap.
 */
int irq_enable(unsigned int irq_no)
{
    /* Call PLIC driver to enable the interrupt */
    plic_irq_enable(irq_no);

    /* Update interrupt tracking bitmap */
    if (!(plic_irq_rec[irq_no >> 5] & (1 << (irq_no % 32)))) {
        plic_irq_rec[irq_no >> 5] |= (1 << (irq_no % 32));
    }
    
    return 0;
}

/**
 * irq_disable - Disable a specific interrupt
 * @irq_no: Interrupt number to disable
 *
 * Returns: 0 on success
 *
 * This function disables the specified interrupt line and updates
 * the interrupt tracking bitmap.
 */
int irq_disable(unsigned int irq_no)
{
    /* Call PLIC driver to disable the interrupt */
    plic_irq_disable(irq_no);

    /* Update interrupt tracking bitmap */
    if (plic_irq_rec[irq_no >> 5] & (1 << (irq_no % 32))) {
        plic_irq_rec[irq_no >> 5] &= ~(1 << (irq_no % 32));
    }
    
    return 0;
}

/**
 * arch_enable_irq - Architecture-specific IRQ enable
 * @irq: Interrupt number
 *
 * This is the architecture-level interface for enabling interrupts.
 */
void arch_enable_irq(int32_t irq)
{
    irq_enable(irq);
}

/**
 * arch_disable_irq - Architecture-specific IRQ disable
 * @irq: Interrupt number
 *
 * This is the architecture-level interface for disabling interrupts.
 */
void arch_disable_irq(int32_t irq)
{
    irq_disable(irq);
}

/**
 * arch_enable_all_irq - Enable all interrupts globally
 *
 * This function enables all interrupts at the processor level.
 */
void arch_enable_all_irq(void)
{
    arch_local_irq_enable();
}

/**
 * arch_disable_all_irq - Disable all interrupts globally
 *
 * This function disables all interrupts at the processor level.
 */
void arch_disable_all_irq(void)
{
    arch_local_irq_disable();
}
