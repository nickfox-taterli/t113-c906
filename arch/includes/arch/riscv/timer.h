/*
 * arch/includes/arch/riscv/timer.h - RISC-V timer handling interface
 */

#ifndef _ARCH_RISCV_TIMER_H
#define _ARCH_RISCV_TIMER_H

/**
 * riscv_timer_interrupt - Machine timer interrupt handler
 */
void riscv_timer_interrupt(void);

/**
 * riscv_system_tick_init - Initialize system tick timer
 */
void riscv_system_tick_init(void);

/**
 * riscv_timer_port_setup - FreeRTOS port timer setup
 */
void riscv_timer_port_setup(void);

#endif /* _ARCH_RISCV_TIMER_H */
