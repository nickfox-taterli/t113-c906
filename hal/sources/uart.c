/*
 * UART HAL driver abstraction layer implementation
 * Based on BSP UART driver implementation
 */

#include "uart.h"
#include "rv_io.h"
#include "delay.h"
#include <string.h>
#include <stdint.h>

/* Timeout definition */
#define UART_TIMEOUT_US      100000  /* 100ms timeout */

/**
 * @brief Initialize UART
 * @param config UART configuration structure pointer
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_init(const uart_config_t *config)
{
    if (config == NULL) {
        return UART_ERROR_PARAM;
    }

    /* Configure data format */
    if (uart_set_format(config->base_addr, config->data_bits,
                         config->stop_bits, config->parity) != UART_OK) {
        return UART_ERROR;
    }

    /* Configure baudrate */
    if (uart_set_baudrate(config->base_addr, config->uart_clk, 
                          config->baudrate) != UART_OK) {
        return UART_ERROR;
    }

    /* Enable FIFO */
    if (uart_enable_fifo(config->base_addr) != UART_OK) {
        return UART_ERROR;
    }

    return UART_OK;
}

/**
 * @brief Deinitialize UART
 * @param config UART configuration structure pointer
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_deinit(const uart_config_t *config)
{
    if (config == NULL) {
        return UART_ERROR_PARAM;
    }

    /* Disable FIFO */
    writeb(0, (volatile void *)((uintptr_t)(config->base_addr + UART_FCR)));

    /* Disable interrupt */
    writeb(0, (volatile void *)((uintptr_t)(config->base_addr + UART_IER)));

    return UART_OK;
}

/**
 * @brief Set baudrate
 * @param base_addr UART base address
 * @param uart_clk UART clock frequency (Hz)
 * @param baudrate Target baudrate
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_set_baudrate(uint32_t base_addr, uint32_t uart_clk, uint32_t baudrate)
{
    uint32_t quot;
    uint8_t lcr, dlh, dll;
    uint32_t timeout;

    if (baudrate == 0) {
        return UART_ERROR_PARAM;
    }

    /* Read current LCR value */
    lcr = readb((volatile void *)((uintptr_t)(base_addr + UART_LCR)));

    /* Calculate baudrate divisor
     * Formula: quot = (uart_clk + 8 * baudrate) / (16 * baudrate)
     * 
     * Example:
     * uart_clk = 24000000 Hz
     * baudrate = 115200
     * quot = (24000000 + 8*115200) / (16*115200) = 13.0
     * UART_DLH = 13 >> 8 = 0x00
     * UART_DLL = 13 & 0xff = 0x0D
     */
    quot = (uart_clk + 8 * baudrate) / (16 * baudrate);

    dlh = quot >> 8;      /* High byte: 0x00 */
    dll = quot & 0xff;    /* Low byte: 0x0D */

    /* 
     * Follow BSP driver configuration flow:
     * 1. Pause transmission, force configuration
     *    HALT = 0x03 (HTX=1, FORCECFG=1)
     */
    writeb(UART_HALT_HTX | UART_HALT_FORCECFG, (volatile void *)((uintptr_t)(base_addr + UART_HALT)));

    /* 
     * 2. Set DLAB to allow access to baudrate registers
     *    LCR = original value | 0x80 (DLAB=1)
     *    Example: 0x03 | 0x80 = 0x83
     */
    writeb(lcr | UART_LCR_DLAB, (volatile void *)((uintptr_t)(base_addr + UART_LCR)));

    /* 
     * 3. Write baudrate registers
     *    DLH = 0x00 (high byte)
     *    DLL = 0x0D (low byte)
     */
    writeb(dlh, (volatile void *)((uintptr_t)(base_addr + UART_DLH)));
    writeb(dll, (volatile void *)((uintptr_t)(base_addr + UART_DLL)));

    /* 
     * 4. Update LCR, set LCRUP bit
     *    HALT = 0x07 (HTX=1, FORCECFG=1, LCRUP=1)
     */
    writeb(UART_HALT_HTX | UART_HALT_FORCECFG | UART_HALT_LCRUP, 
               (volatile void *)((uintptr_t)(base_addr + UART_HALT)));

    /* 
     * 5. Wait for LCRUP bit to clear (indicates LCR update complete)
     *    Hardware will automatically clear this bit after configuration is complete
     */
    timeout = UART_TIMEOUT_US;
    while (readb((volatile void *)((uintptr_t)(base_addr + UART_HALT))) & UART_HALT_LCRUP) {
        if (timeout-- == 0) {
            return UART_ERROR_TIMEOUT;
        }
        udelay(1);
    }

    /* 
     * 6. Restore LCR (clear DLAB)
     *    LCR = original value
     *    Example: 0x03 (DLAB=0)
     */
    writeb(lcr, (volatile void *)((uintptr_t)(base_addr + UART_LCR)));

    /* 
     * 7. Keep force configuration mode
     *    HALT = 0x02 (FORCECFG=1)
     *    Note: FORCECFG is always 1, can choose DLAB_BAK,
     *    so configuration can be done regardless of whether UART is busy
     */
    writeb(UART_HALT_FORCECFG, (volatile void *)((uintptr_t)(base_addr + UART_HALT)));

    return UART_OK;
}

/**
 * @brief Set data format (data bits, stop bits, parity)
 * @param base_addr UART base address
 * @param data_bits Number of data bits (5-8)
 * @param stop_bits Number of stop bits (1-2)
 * @param parity Parity bit (0=none, 1=odd, 2=even)
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_set_format(uint32_t base_addr, uint8_t data_bits,
                              uint8_t stop_bits, uint8_t parity)
{
    uint8_t lcr = 0;

    /* Set number of data bits */
    switch (data_bits) {
        case 5:
            lcr |= UART_LCR_WLEN5;
            break;
        case 6:
            lcr |= UART_LCR_WLEN6;
            break;
        case 7:
            lcr |= UART_LCR_WLEN7;
            break;
        case 8:
        default:
            lcr |= UART_LCR_WLEN8;
            break;
    }

    /* Set number of stop bits */
    switch (stop_bits) {
        case 2:
            lcr |= UART_LCR_STOP;
            break;
        case 1:
        default:
            lcr &= ~UART_LCR_STOP;
            break;
    }

    /* Set parity bit */
    switch (parity) {
        case 1:  /* Odd parity */
            lcr |= UART_LCR_PARITY;
            lcr &= ~UART_LCR_EPAR;
            break;
        case 2:  /* Even parity */
            lcr |= UART_LCR_PARITY | UART_LCR_EPAR;
            break;
        case 0:  /* No parity */
        default:
            lcr &= ~UART_LCR_PARITY;
            break;
    }

    /* Write to LCR register */
    writeb(lcr, (volatile void *)((uintptr_t)(base_addr + UART_LCR)));

    return UART_OK;
}

/**
 * @brief Enable FIFO
 * @param base_addr UART base address
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_enable_fifo(uint32_t base_addr)
{
    /* 
     * Configure FIFO:
     * - FIFO enable
     * - Receive FIFO reset
     * - Transmit FIFO reset
     * - Receive FIFO trigger 1/2
     * - Transmit FIFO trigger 1/2
     * FCR = 0x01 | 0x02 | 0x04 | (0x2 << 6) | (0x2 << 4) = 0xC7
     */
    uint8_t fcr = UART_FCR_FIFO_EN | UART_FCR_RXFIFO_RST | UART_FCR_TXFIFO_RST |
                   UART_FCR_RXTRG_1_2 | UART_FCR_TXTRG_1_2;

    /* Write to FCR register */
    writeb(fcr, (volatile void *)((uintptr_t)(base_addr + UART_FCR)));

    /* Ensure FIFO configuration takes effect, read back once */
    readb((volatile void *)((uintptr_t)(base_addr + UART_FCR)));

    return UART_OK;
}

/**
 * @brief Send single byte
 * @param base_addr UART base address
 * @param ch Byte to send
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_send_byte(uint32_t base_addr, uint8_t ch)
{
    uint32_t timeout = UART_TIMEOUT_US;

    /* Wait for transmit holding register empty */
    while (!(readb((volatile void *)((uintptr_t)(base_addr + UART_LSR))) & UART_LSR_THRE)) {
        if (timeout-- == 0) {
            return UART_ERROR_TIMEOUT;
        }
        udelay(1);
    }

    /* Write to transmit holding register */
    writeb(ch, (volatile void *)((uintptr_t)(base_addr + UART_THR)));

    return UART_OK;
}

/**
 * @brief Receive single byte (blocking mode)
 * @param base_addr UART base address
 * @param ch Pointer to received byte
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_recv_byte(uint32_t base_addr, uint8_t *ch)
{
    uint32_t timeout = UART_TIMEOUT_US;

    if (ch == NULL) {
        return UART_ERROR_PARAM;
    }

    /* Wait for data ready */
    while (!(readb((volatile void *)((uintptr_t)(base_addr + UART_LSR))) & UART_LSR_DR)) {
        if (timeout-- == 0) {
            return UART_ERROR_TIMEOUT;
        }
        udelay(1);
    }

    /* Read receive buffer register */
    *ch = readb((volatile void *)((uintptr_t)(base_addr + UART_RBR)));

    return UART_OK;
}

/**
 * @brief Send data
 * @param base_addr UART base address
 * @param data Data pointer
 * @param len Data length
 * @return Actual number of bytes sent, negative value indicates error
 */
int uart_send(uint32_t base_addr, const uint8_t *data, size_t len)
{
    size_t i;
    int sent = 0;

    if (data == NULL || len == 0) {
        return UART_ERROR_PARAM;
    }

    for (i = 0; i < len; i++) {
        if (uart_send_byte(base_addr, data[i]) == UART_OK) {
            sent++;
        } else {
            break;
        }
    }

    return sent;
}

/**
 * @brief Receive data (blocking mode)
 * @param base_addr UART base address
 * @param data Receive buffer pointer
 * @param len Data length to receive
 * @return Actual number of bytes received, negative value indicates error
 */
int uart_recv(uint32_t base_addr, uint8_t *data, size_t len)
{
    size_t i;
    int received = 0;

    if (data == NULL || len == 0) {
        return UART_ERROR_PARAM;
    }

    for (i = 0; i < len; i++) {
        if (uart_recv_byte(base_addr, &data[i]) == UART_OK) {
            received++;
        } else {
            break;
        }
    }

    return received;
}

/**
 * @brief Check if transmit FIFO is empty
 * @param base_addr UART base address
 * @return 1=empty, 0=not empty
 */
int uart_is_tx_empty(uint32_t base_addr)
{
    uint8_t usr = readb((volatile void *)((uintptr_t)(base_addr + UART_USR)));
    return (usr & UART_USR_TFE) ? 1 : 0;
}

/**
 * @brief Check if receive FIFO has data
 * @param base_addr UART base address
 * @return 1=has data, 0=no data
 */
int uart_is_rx_ready(uint32_t base_addr)
{
    uint8_t usr = readb((volatile void *)((uintptr_t)(base_addr + UART_USR)));
    return (usr & UART_USR_RFNE) ? 1 : 0;
}
