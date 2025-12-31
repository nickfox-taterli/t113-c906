#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Delay factor definition
 * Assuming CPU frequency is 800MHz
 * Cycles needed per microsecond = 800MHz * 1us = 800 cycles
 * Considering loop overhead, use 200 as delay factor
 */
#define UDELAY_FACTOR    200

/**
 * @brief Microsecond delay function
 * @param us Delay time in microseconds
 */
void udelay(uint32_t us);

/**
 * @brief Millisecond delay function
 * @param ms Delay time in milliseconds
 */
void mdelay(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* DELAY_H */
