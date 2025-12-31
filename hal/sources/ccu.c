#include "ccu.h"
#include "delay.h"
#include <stdint.h>

/* Register access macro - use uintptr_t to avoid type conversion warnings */
#define CCU_REG(offset)      (*(volatile uint32_t *)((uintptr_t)(CCU_BASE_ADDR) + (offset)))

/* Bit operation macros */
#define BIT_SET(reg, bit)    ((reg) |= (1U << (bit)))
#define BIT_CLR(reg, bit)    ((reg) &= ~(1U << (bit)))
#define BIT_GET(reg, bit)    (((reg) >> (bit)) & 0x1)

/* Parent clock ID definitions */
#define PARENT_CLK_PSI_AHB   0xFF  /* psi-ahb parent clock */
#define PARENT_CLK_APB1      0xFE  /* apb1 parent clock */

/* Clock configuration table - flat format */
static const clk_cfg_t clk_table[CLK_MAX] = {
    /* MMC clocks */
    {"bus-mmc0",  0x84c, 0,  PARENT_CLK_PSI_AHB, true},
    {"bus-mmc1",  0x84c, 1,  PARENT_CLK_PSI_AHB, true},
    {"bus-mmc2",  0x84c, 2,  PARENT_CLK_PSI_AHB, true},
    
    /* UART clocks */
    {"bus-uart0", 0x90c, 0,  PARENT_CLK_APB1,    true},
    {"bus-uart1", 0x90c, 1,  PARENT_CLK_APB1,    true},
    {"bus-uart2", 0x90c, 2,  PARENT_CLK_APB1,    true},
    {"bus-uart3", 0x90c, 3,  PARENT_CLK_APB1,    true},
    {"bus-uart4", 0x90c, 4,  PARENT_CLK_APB1,    true},
    {"bus-uart5", 0x90c, 5,  PARENT_CLK_APB1,    true},
    
    /* I2C clocks */
    {"bus-i2c0",  0x91c, 0,  PARENT_CLK_APB1,    true},
    {"bus-i2c1",  0x91c, 1,  PARENT_CLK_APB1,    true},
    {"bus-i2c2",  0x91c, 2,  PARENT_CLK_APB1,    true},
    {"bus-i2c3",  0x91c, 3,  PARENT_CLK_APB1,    true},
};

/* Reset configuration structure */
typedef struct {
    uint32_t reg_offset;        /* Register offset */
    uint32_t bit_pos;           /* Reset bit position */
} rst_cfg_t;

/* Reset configuration table - flat format */
static const rst_cfg_t rst_table[RST_MAX] = {
    /* MMC resets */
    {0x2c0, 16},  /* RST_MMC0 */
    {0x2c0, 17},  /* RST_MMC1 */
    {0x2c0, 18},  /* RST_MMC2 */
    
    /* UART resets */
    {0x2d8, 16},  /* RST_UART0 */
    {0x2d8, 17},  /* RST_UART1 */
    {0x2d8, 18},  /* RST_UART2 */
    {0x2d8, 19},  /* RST_UART3 */
    {0x2d8, 20},  /* RST_UART4 */
    {0x2d8, 21},  /* RST_UART5 */
    
    /* I2C resets */
    {0x2d0, 0},   /* RST_I2C0 */
    {0x2d0, 1},   /* RST_I2C1 */
    {0x2d0, 2},   /* RST_I2C2 */
    {0x2d0, 3},   /* RST_I2C3 */
};


/**
 * @brief Enable clock
 * @param clk_id Clock ID
 */
void CCU_EnableClock(clk_id_t clk_id)
{
    uint32_t reg_val;
    
    if (clk_id >= CLK_MAX) {
        return;
    }
    
    /* Read current register value */
    reg_val = CCU_REG(clk_table[clk_id].reg_offset);
    
    /* Set corresponding enable bit */
    BIT_SET(reg_val, clk_table[clk_id].bit_pos);
    
    /* Write back to register */
    CCU_REG(clk_table[clk_id].reg_offset) = reg_val;
}

/**
 * @brief Disable clock
 * @param clk_id Clock ID
 */
void CCU_DisableClock(clk_id_t clk_id)
{
    uint32_t reg_val;
    
    if (clk_id >= CLK_MAX) {
        return;
    }
    
    /* Read current register value */
    reg_val = CCU_REG(clk_table[clk_id].reg_offset);
    
    /* Clear corresponding enable bit */
    BIT_CLR(reg_val, clk_table[clk_id].bit_pos);
    
    /* Write back to register */
    CCU_REG(clk_table[clk_id].reg_offset) = reg_val;
}

/**
 * @brief Check if clock is enabled
 * @param clk_id Clock ID
 * @return true if enabled, false if disabled
 */
bool CCU_IsClockEnabled(clk_id_t clk_id)
{
    uint32_t reg_val;
    
    if (clk_id >= CLK_MAX) {
        return false;
    }
    
    /* Read register value */
    reg_val = CCU_REG(clk_table[clk_id].reg_offset);
    
    /* Return value of corresponding bit */
    return BIT_GET(reg_val, clk_table[clk_id].bit_pos) ? true : false;
}

/**
 * @brief Set clock frequency
 * @param clk_id Clock ID
 * @param rate Target frequency (Hz)
 * 
 * Note: For simple gate clocks, this function may not apply
 * Need to implement specific frequency setting logic based on actual hardware
 */
void CCU_SetClockRate(clk_id_t clk_id, uint32_t rate)
{
    /* For bus clocks, usually gate control, does not support frequency setting */
    /* If frequency setting is needed, need to implement corresponding multiplexer and divider logic */
    
    (void)clk_id;
    (void)rate;
    
    /* Not implemented for now */
}

/**
 * @brief Get clock frequency
 * @param clk_id Clock ID
 * @return Clock frequency (Hz)
 * 
 * Note: For simple gate clocks, return parent clock frequency
 */
uint32_t CCU_GetClockRate(clk_id_t clk_id)
{
    /* For bus clocks, return parent clock frequency */
    /* psi-ahb: 600MHz (typical value, actual depends on hardware configuration) */
    /* apb1: psi-ahb / N (typical value, depends on hardware configuration) */
    
    if (clk_id >= CLK_MAX) {
        return 0;
    }
    
    /* Return typical frequency value based on parent clock type */
    if (clk_table[clk_id].parent_clk_id == PARENT_CLK_PSI_AHB) {
        return 600000000;  /* psi-ahb: 600MHz */
    } else if (clk_table[clk_id].parent_clk_id == PARENT_CLK_APB1) {
        return 24000000;   /* apb1: 24MHz (typical value) */
    }
    
    return 0;
}

/**
 * @brief Assert reset (enable reset signal)
 * @param rst_id Reset ID
 * 
 * Note: In hardware reset control, bit 0 means reset is active
 */
void CCU_ResetAssert(rst_id_t rst_id)
{
    uint32_t reg_val;
    
    if (rst_id >= RST_MAX) {
        return;
    }
    
    /* Read current register value */
    reg_val = CCU_REG(rst_table[rst_id].reg_offset);
    
    /* Clear corresponding reset bit (assert reset) */
    BIT_CLR(reg_val, rst_table[rst_id].bit_pos);
    
    /* Write back to register */
    CCU_REG(rst_table[rst_id].reg_offset) = reg_val;
}

/**
 * @brief Deassert reset (release reset signal)
 * @param rst_id Reset ID
 * 
 * Note: In hardware reset control, bit 1 means reset is released
 */
void CCU_ResetDeassert(rst_id_t rst_id)
{
    uint32_t reg_val;
    
    if (rst_id >= RST_MAX) {
        return;
    }
    
    /* Read current register value */
    reg_val = CCU_REG(rst_table[rst_id].reg_offset);
    
    /* Set corresponding reset bit (release reset) */
    BIT_SET(reg_val, rst_table[rst_id].bit_pos);
    
    /* Write back to register */
    CCU_REG(rst_table[rst_id].reg_offset) = reg_val;
}

/**
 * @brief Perform reset operation
 * @param rst_id Reset ID
 * 
 * This function will first assert reset, wait for a period, then release reset
 */
void CCU_Reset(rst_id_t rst_id)
{
    if (rst_id >= RST_MAX) {
        return;
    }
    
    /* First assert reset */
    CCU_ResetAssert(rst_id);
    
    /* Delay wait for reset to take effect */
    udelay(10);
    
    /* Release reset */
    CCU_ResetDeassert(rst_id);
}

/**
 * @brief Get reset status
 * @param rst_id Reset ID
 * @return true if reset is active (asserted), false if reset is released
 * 
 * Note: In hardware reset control, bit 0 means reset is active, bit 1 means reset is released
 *       But function return value semantics: true means reset is active, false means reset is released
 */
bool CCU_GetResetStatus(rst_id_t rst_id)
{
    uint32_t reg_val;
    
    if (rst_id >= RST_MAX) {
        return false;
    }
    
    /* Read register value */
    reg_val = CCU_REG(rst_table[rst_id].reg_offset);
    
    /* Return reset status: bit 0 means reset is active */
    return !(BIT_GET(reg_val, rst_table[rst_id].bit_pos));
}
