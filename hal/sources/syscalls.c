#include <reent.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "uart.h"
#include "delay.h"

/* UART base address definition */
#define UART_BASE    SUNXI_UART2_BASE

/**
 * @brief Write system call override - for output functions like printf
 * @param ptr Reentrant structure pointer
 * @param fd File descriptor (0=stdin, 1=stdout, 2=stderr)
 * @param buf Data buffer
 * @param nbytes Number of bytes to write
 * @return Actual number of bytes written
 */
_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t nbytes)
{
    /* Only handle standard output and standard error */
    if (fd < 3) {
        const char *b = (const char *)buf;
        int i;
        
        /* Iterate through buffer, send each character */
        for (i = 0; i < nbytes; i++) {
            /* Convert newline to carriage return + newline */
            if (b[i] == '\n') {
                uart_send_byte(UART_BASE, '\r');
            }
            uart_send_byte(UART_BASE, (uint8_t)b[i]);
        }
        
        return nbytes;
    }
    
    /* Other file descriptors return error */
    ptr->_errno = ENOSYS;
    return -1;
}

/**
 * @brief Read system call override - for input functions like scanf
 * @param ptr Reentrant structure pointer
 * @param fd File descriptor (0=stdin, 1=stdout, 2=stderr)
 * @param buf Data buffer
 * @param nbytes Number of bytes to read
 * @return Actual number of bytes read
 */
_ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t nbytes)
{
    /* Only handle standard input */
    if (fd < 3) {
        char *b = (char *)buf;
        int i;
        uint8_t ch;
        int ret;
        
        /* Read specified number of bytes from UART */
        for (i = 0; i < nbytes; i++) {
            ret = uart_recv_byte(UART_BASE, &ch);
            if (ret != 0) {
                /* Read failed, return number of bytes read */
                break;
            }
            b[i] = (char)ch;
        }
        
        return i;
    }
    
    /* Other file descriptors return error */
    ptr->_errno = ENOSYS;
    return -1;
}

/**
 * @brief Close file descriptor
 * @param ptr Reentrant structure pointer
 * @param fd File descriptor
 * @return 0 success, -1 failure
 */
int _close_r(struct _reent *ptr, int fd)
{
    /* Standard input/output/error cannot be closed */
    if (fd < 3) {
        ptr->_errno = EINVAL;
        return -1;
    }
    
    ptr->_errno = ENOSYS;
    return -1;
}

/**
 * @brief File status query
 * @param ptr Reentrant structure pointer
 * @param fd File descriptor
 * @param pstat Status structure pointer
 * @return 0 success, -1 failure
 */
int _fstat_r(struct _reent *ptr, int fd, struct stat *pstat)
{
    /* Standard input/output/error treated as character device */
    if (fd < 3) {
        pstat->st_mode = S_IFCHR;
        return 0;
    }
    
    ptr->_errno = ENOSYS;
    return -1;
}

/**
 * @brief Check if it is a terminal device
 * @param ptr Reentrant structure pointer
 * @param fd File descriptor
 * @return 1 if terminal, 0 if not
 */
int _isatty_r(struct _reent *ptr, int fd)
{
    /* Standard input/output/error are terminal devices */
    if (fd >= 0 && fd < 3) {
        return 1;
    }
    
    ptr->_errno = ENOTSUP;
    return 0;
}

/**
 * @brief Move file read/write position
 * @param ptr Reentrant structure pointer
 * @param fd File descriptor
 * @param pos Offset
 * @param whence Starting position
 * @return New file position
 */
_off_t _lseek_r(struct _reent *ptr, int fd, _off_t pos, int whence)
{
    /* Standard input/output/error do not support seeking */
    if (fd < 3) {
        ptr->_errno = ESPIPE;
        return -1;
    }
    
    ptr->_errno = ENOSYS;
    return -1;
}

/**
 * @brief Increase heap space
 * @param ptr Reentrant structure pointer
 * @param incr Number of bytes to increase
 * @return Previous heap top pointer
 */
void *_sbrk_r(struct _reent *ptr, ptrdiff_t incr)
{
    /* Static heap space */
    extern uint8_t __bss_end__;
    extern uint8_t __heap_end__;
    
    static void *heap_end = NULL;
    void *prev_heap_end;
    
    /* Initialize heap pointer */
    if (heap_end == NULL) {
        heap_end = (void *)&__bss_end__;
    }
    
    prev_heap_end = heap_end;
    
    /* Check if exceeds heap space */
    if (heap_end + incr > (void *)&__heap_end__) {
        ptr->_errno = ENOMEM;
        return (void *)-1;
    }
    
    heap_end += incr;
    return prev_heap_end;
}
