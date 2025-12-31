#ifndef CLIC_H
#define CLIC_H

#include <stdint.h>

/**
 * @brief CLIC interrupt control register base address
 */
extern unsigned long clic_int_control_addr_base;

/**
 * @brief CLIC mtime register address
 */
extern unsigned long clic_int_control_addr_mtime;

/**
 * @brief CLIC mtimecmp register address
 */
extern unsigned long clic_int_control_addr_comp;

/**
 * @brief CLIC driver initialization
 * @return 0 indicates success
 */
int clic_driver_init(void);

#endif /* CLIC_H */
