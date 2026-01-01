#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

enum log_level {
    LOG_LEVEL_EMERG = 0,
    LOG_LEVEL_ALERT,
    LOG_LEVEL_CRIT,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};

/* Compile-time default log level; override in build or before including. */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

void log_printf(enum log_level level, const char *tag, const char *fmt, ...);

#ifndef LOG_TAG
#define LOG_TAG "APP"
#endif

#define LOGE(fmt, ...) log_printf(LOG_LEVEL_ERROR, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) log_printf(LOG_LEVEL_WARN, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) log_printf(LOG_LEVEL_INFO, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) log_printf(LOG_LEVEL_DEBUG, LOG_TAG, fmt, ##__VA_ARGS__)

#endif /* LOG_H */
