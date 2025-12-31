#ifndef __CCU_H
#define __CCU_H

#include <stdint.h>
#include <stdbool.h>

/* CCU base address */
#define CCU_BASE_ADDR  (0x02001000)

/* Clock ID enumeration */
typedef enum {
    /* MMC clocks */
    BUS_MMC0 = 0,
    BUS_MMC1,
    BUS_MMC2,
    
    /* UART clocks */
    BUS_UART0,
    BUS_UART1,
    BUS_UART2,
    BUS_UART3,
    BUS_UART4,
    BUS_UART5,
    
    /* I2C clocks */
    BUS_I2C0,
    BUS_I2C1,
    BUS_I2C2,
    BUS_I2C3,
    
    /* Clock count */
    CLK_MAX
} clk_id_t;

/* Reset ID enumeration */
typedef enum {
    /* MMC resets */
    RST_MMC0 = 0,
    RST_MMC1,
    RST_MMC2,
    
    /* UART resets */
    RST_UART0,
    RST_UART1,
    RST_UART2,
    RST_UART3,
    RST_UART4,
    RST_UART5,
    
    /* I2C resets */
    RST_I2C0,
    RST_I2C1,
    RST_I2C2,
    RST_I2C3,
    
    /* Reset count */
    RST_MAX
} rst_id_t;

/* Clock configuration structure */
typedef struct {
    const char *name;           /* Clock name */
    uint32_t reg_offset;        /* Register offset */
    uint32_t bit_pos;           /* Enable bit position */
    uint32_t parent_clk_id;     /* Parent clock ID (if needed) */
    bool has_parent;            /* Has parent clock */
} clk_cfg_t;

void CCU_EnableClock(clk_id_t clk_id);
void CCU_DisableClock(clk_id_t clk_id);
bool CCU_IsClockEnabled(clk_id_t clk_id);
void CCU_SetClockRate(clk_id_t clk_id, uint32_t rate);
uint32_t CCU_GetClockRate(clk_id_t clk_id);

/* Reset control functions */
void CCU_ResetAssert(rst_id_t rst_id);
void CCU_ResetDeassert(rst_id_t rst_id);
void CCU_Reset(rst_id_t rst_id);
bool CCU_GetResetStatus(rst_id_t rst_id);

#endif /* __CCU_H */
