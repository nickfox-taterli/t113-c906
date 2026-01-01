/*
 * arch/riscv/irq.c - RISC-V interrupt management
 *
 * This file contains architecture-level interrupt management functions
 * for the C906 RISC-V processor.
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "csr.h"
#include "irqflags.h"
#include "irqs.h"
#include "plic.h"
#include "irq.h"

#define LOG_TAG "irq"
#include "log.h"

struct irq_handler_entry {
    irq_handler_t handler;
    void *param;
    const char *name;
};

static struct irq_handler_entry irq_table[NR_IRQS + 1];

/* 
 * Interrupt tracking bitmap for PLIC interrupts
 * 32-bit words, each bit represents one IRQ line
 */
uint32_t plic_irq_rec[(NR_IRQS + 1 + 32) >> 5] = {0};

static inline void record_irq_enable(unsigned int irq_no)
{
    if (!(plic_irq_rec[irq_no >> 5] & (1u << (irq_no % 32)))) {
        plic_irq_rec[irq_no >> 5] |= (1u << (irq_no % 32));
    }
}

static inline void record_irq_disable(unsigned int irq_no)
{
    if (plic_irq_rec[irq_no >> 5] & (1u << (irq_no % 32))) {
        plic_irq_rec[irq_no >> 5] &= ~(1u << (irq_no % 32));
    }
}

int irq_request(unsigned int irq_no, irq_handler_t handler, void *param, const char *name)
{
    if (irq_no >= NR_IRQS || handler == NULL) {
        return -1;
    }

    irq_table[irq_no].handler = handler;
    irq_table[irq_no].param = param;
    irq_table[irq_no].name = name ? name : "anon";

    /* Allow external interrupts globally */
    csr_set(mie, MIE_MEIE);
    arch_local_irq_enable();

    /* Enable in PLIC */
    plic_irq_enable(irq_no);
    record_irq_enable(irq_no);

    LOGI("irq %u (%s) registered and enabled", irq_no, irq_table[irq_no].name);
    return 0;
}

int irq_free(unsigned int irq_no)
{
    if (irq_no >= NR_IRQS) {
        return -1;
    }

    plic_irq_disable(irq_no);
    record_irq_disable(irq_no);
    memset(&irq_table[irq_no], 0, sizeof(irq_table[irq_no]));
    LOGI("irq %u freed", irq_no);
    return 0;
}

/* 
 * Generic IRQ handler - dispatch to registered handlers
 */
int generic_handle_irq(unsigned int irq)
{
    if (irq >= NR_IRQS) {
        LOGW("spurious IRQ %u (>= NR_IRQS)", irq);
        return -1;
    }

    struct irq_handler_entry *entry = &irq_table[irq];
    if (entry->handler) {
        entry->handler(entry->param);
    } else {
        LOGW("unhandled IRQ %u", irq);
    }

    /* Always signal completion to the PLIC */
    plic_irq_eoi(irq);
    return 0;
}

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
    record_irq_enable(irq_no);
    
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
    record_irq_disable(irq_no);
    
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
