#include "delay.h"
#include <stdint.h>

/**
 * @brief Microsecond delay function
 * @param us Delay time in microseconds
 */
void udelay(uint32_t us)
{
    volatile uint32_t i;
    us *= UDELAY_FACTOR;  /* Use delay factor */
    for (i = 0; i < us; i++) {
        __asm__ volatile("nop");
    }
}

/**
 * @brief Millisecond delay function
 * @param ms Delay time in milliseconds
 */
void mdelay(uint32_t ms)
{
    /* 1 millisecond = 1000 microseconds */
    udelay(ms * 1000);
}
