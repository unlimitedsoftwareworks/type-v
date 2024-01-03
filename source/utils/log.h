// my_logger.h
#ifndef TYPVM_LOGGER_H
#define TYPVM_LOGGER_H

#include <stdio.h>
#include <stdarg.h>

// Color definitions
#define LOG_COLOR_RED     "\x1b[31m"
#define LOG_COLOR_GREEN   "\x1b[32m"
#define LOG_COLOR_YELLOW  "\x1b[33m"
#define LOG_COLOR_BLUE    "\x1b[34m"
#define LOG_COLOR_MAGENTA "\x1b[35m"
#define LOG_COLOR_CYAN    "\x1b[36m"
#define LOG_COLOR_RESET   "\x1b[0m"

// Internal logging function
static inline void log_internal(const char *color, const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s", color);
    vprintf(format, args);
    printf("%s\n", LOG_COLOR_RESET);
    va_end(args);
}

// Public API Macros
#define LOG_INFO(fmt, ...)//    log_internal(LOG_COLOR_CYAN,    "[INFO] " fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)//    log_internal(LOG_COLOR_YELLOW,  "[WARN] " fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   log_internal(LOG_COLOR_RED,     "[ERROR] " fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)   log_internal(LOG_COLOR_GREEN,   "[DEBUG] " fmt, ##__VA_ARGS__)

#endif
