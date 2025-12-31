#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "csr.h"
#include "rv_io.h"
#include "clic.h"

/* CLINT related address definitions */
#define C910_PLIC_CLINT_OFFSET    0x04000000

/* Global variable declarations */
unsigned long clic_int_control_addr_base = 0;
unsigned long clic_int_control_addr_mtime = 0;
unsigned long clic_int_control_addr_comp = 0;

int clic_driver_init(void)
{
    csr_set(mie, MIE_MSIE);

    clic_int_control_addr_base = csr_read(CSR_PLIC_BASE) + C910_PLIC_CLINT_OFFSET;
    if (clic_int_control_addr_base == 0) {
        printf("clic_driver_init: CSR_PLIC_BASE is zero!\n");
    }
    clic_int_control_addr_mtime = clic_int_control_addr_base + 0xbff8;
    clic_int_control_addr_comp = clic_int_control_addr_base + 0x4000;

    return 0;
}
