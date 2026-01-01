#include "log.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

static const char *const log_level_names[] = {
    "EMERG",
    "ALERT",
    "CRIT",
    "ERR",
    "WARN",
    "NOTICE",
    "INFO",
    "DEBUG"
};

static unsigned long log_get_time_ms(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) {
        return 0;
    }

    return (unsigned long)((uint64_t)xTaskGetTickCount() * 1000ULL /
                           (uint64_t)configTICK_RATE_HZ);
}

void log_printf(enum log_level level, const char *tag, const char *fmt, ...)
{
    va_list args;
    unsigned long ms;
    unsigned long sec;
    unsigned long msec;
    size_t fmt_len;

    if (level > LOG_LEVEL || level >= (int)(sizeof(log_level_names) / sizeof(log_level_names[0]))) {
        return;
    }

    ms = log_get_time_ms();
    sec = ms / 1000U;
    msec = ms % 1000U;

    if (!tag) {
        tag = "NULL";
    }

    printf("[  %6lu.%03lu] %s: ", sec, msec, tag);

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fmt_len = fmt ? strlen(fmt) : 0;
    if (fmt_len == 0 || fmt[fmt_len - 1] != '\n') {
        printf("\n");
    }
}