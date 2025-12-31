/*
 * UART HAL driver abstraction layer header file
 * Based on BSP UART driver implementation
 */

#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <stddef.h>

#define SUNXI_UART0_BASE	    (0x02500000)
#define SUNXI_UART1_BASE	    (0x02500400)
#define SUNXI_UART2_BASE	    (0x02500800)
#define SUNXI_UART3_BASE	    (0x02500C00)
#define SUNXI_UART4_BASE	    (0x02501000)
#define SUNXI_UART5_BASE	    (0x02501400)

/* UART register definitions */
#define UART_RBR (0x00)         /* Receive buffer register */
#define UART_THR (0x00)         /* Transmit holding register */
#define UART_DLL (0x00)         /* Divisor latch low byte */
#define UART_DLH (0x04)         /* Divisor latch high byte */
#define UART_IER (0x04)         /* Interrupt enable register */
#define UART_IIR (0x08)         /* Interrupt identification register */
#define UART_FCR (0x08)         /* FIFO control register */
#define UART_LCR (0x0c)         /* Line control register */
#define UART_MCR (0x10)         /* Modem control register */
#define UART_LSR (0x14)         /* Line status register */
#define UART_MSR (0x18)         /* Modem status register */
#define UART_USR (0x7c)         /* Status register */
#define UART_HALT (0xA4)        /* Halt transmit register */

/* LCR register bit definitions */
#define UART_LCR_DLAB       (0x1 << 7)  /* Divisor latch access bit */
#define UART_LCR_SBC        (0x1 << 6)  /* Set break control */
#define UART_LCR_EPAR       (0x1 << 4)  /* Even parity select */
#define UART_LCR_PARITY     (0x1 << 3)  /* Parity enable */
#define UART_LCR_STOP       (0x1 << 2)  /* Stop bit */
#define UART_LCR_WLEN8      (0x3)       /* 8-bit data length */
#define UART_LCR_WLEN7      (0x2)       /* 7-bit data length */
#define UART_LCR_WLEN6      (0x1)       /* 6-bit data length */
#define UART_LCR_WLEN5      (0x0)       /* 5-bit data length */

/* FCR register bit definitions */
#define UART_FCR_FIFO_EN    (0x1 << 0)  /* FIFO enable */
#define UART_FCR_RXFIFO_RST (0x1 << 1)  /* Receive FIFO reset */
#define UART_FCR_TXFIFO_RST (0x1 << 2)  /* Transmit FIFO reset */
#define UART_FCR_RXTRG_1_2  (0x2 << 6)  /* Receive FIFO trigger 1/2 */
#define UART_FCR_TXTRG_1_2  (0x2 << 4)  /* Transmit FIFO trigger 1/2 */

/* IER register bit definitions */
#define UART_IER_RDI        (0x1 << 0)  /* Receive data interrupt enable */
#define UART_IER_THRI       (0x1 << 1)  /* Transmit holding register empty interrupt enable */
#define UART_IER_RLSI       (0x1 << 2)  /* Receiver line status interrupt enable */
#define UART_IER_MSI        (0x1 << 3)  /* Modem status interrupt enable */

/* HALT register bit definitions */
#define UART_HALT_HTX       (0x1 << 0)  /* Halt transmit */
#define UART_HALT_FORCECFG  (0x1 << 1)  /* Force configuration */
#define UART_HALT_LCRUP     (0x1 << 2)  /* LCR update */

/* LSR register bit definitions */
#define UART_LSR_DR         (0x1 << 0)  /* Data ready */
#define UART_LSR_THRE       (0x1 << 5)  /* Transmit holding register empty */
#define UART_LSR_TEMT       (0x1 << 6)  /* Transmitter empty */

/* USR register bit definitions */
#define UART_USR_TFE        (0x1 << 2)  /* Transmit FIFO empty */
#define UART_USR_RFNE       (0x1 << 3)  /* Receive FIFO not empty */
#define UART_USR_BUSY       (0x1 << 0)  /* UART busy */

/* UART configuration structure */
typedef struct {
    uint32_t base_addr;     /* UART base address */
    uint32_t uart_clk;      /* UART clock frequency (Hz) */
    uint32_t baudrate;      /* Baudrate */
    uint8_t  data_bits;     /* Number of data bits (5-8) */
    uint8_t  stop_bits;     /* Number of stop bits (1-2) */
    uint8_t  parity;        /* Parity bit (0=none, 1=odd, 2=even) */
} uart_config_t;

/* Standard baudrate definitions */
typedef enum {
    UART_BAUD_9600    = 9600,
    UART_BAUD_19200   = 19200,
    UART_BAUD_38400   = 38400,
    UART_BAUD_57600   = 57600,
    UART_BAUD_115200  = 115200,
    UART_BAUD_230400  = 230400,
    UART_BAUD_460800  = 460800,
    UART_BAUD_921600  = 921600,
} uart_baudrate_t;

/* UART error codes */
typedef enum {
    UART_OK          = 0,
    UART_ERROR       = -1,
    UART_ERROR_PARAM = -2,
    UART_ERROR_TIMEOUT = -3,
} uart_error_t;

/* Function declarations */

/**
 * @brief Initialize UART
 * @param config UART configuration structure pointer
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_init(const uart_config_t *config);

/**
 * @brief Deinitialize UART
 * @param config UART configuration structure pointer
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_deinit(const uart_config_t *config);

/**
 * @brief Set baudrate
 * @param base_addr UART base address
 * @param uart_clk UART clock frequency (Hz)
 * @param baudrate Target baudrate
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_set_baudrate(uint32_t base_addr, uint32_t uart_clk, uint32_t baudrate);

/**
 * @brief Set data format (data bits, stop bits, parity)
 * @param base_addr UART base address
 * @param data_bits Number of data bits (5-8)
 * @param stop_bits Number of stop bits (1-2)
 * @param parity Parity bit (0=none, 1=odd, 2=even)
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_set_format(uint32_t base_addr, uint8_t data_bits,
                              uint8_t stop_bits, uint8_t parity);

/**
 * @brief Enable FIFO
 * @param base_addr UART base address
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_enable_fifo(uint32_t base_addr);

/**
 * @brief Send single byte
 * @param base_addr UART base address
 * @param ch Byte to send
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_send_byte(uint32_t base_addr, uint8_t ch);

/**
 * @brief Receive single byte (blocking mode)
 * @param base_addr UART base address
 * @param ch Pointer to received byte
 * @return UART_OK on success, other values on failure
 */
uart_error_t uart_recv_byte(uint32_t base_addr, uint8_t *ch);

/**
 * @brief Send data
 * @param base_addr UART base address
 * @param data Data pointer
 * @param len Data length
 * @return Actual number of bytes sent, negative value indicates error
 */
int uart_send(uint32_t base_addr, const uint8_t *data, size_t len);

/**
 * @brief Receive data (blocking mode)
 * @param base_addr UART base address
 * @param data Receive buffer pointer
 * @param len Data length to receive
 * @return Actual number of bytes received, negative value indicates error
 */
int uart_recv(uint32_t base_addr, uint8_t *data, size_t len);

/**
 * @brief Check if transmit FIFO is empty
 * @param base_addr UART base address
 * @return 1=empty, 0=not empty
 */
int uart_is_tx_empty(uint32_t base_addr);

/**
 * @brief Check if receive FIFO has data
 * @param base_addr UART base address
 * @return 1=has data, 0=no data
 */
int uart_is_rx_ready(uint32_t base_addr);

#endif /* __UART_H */
