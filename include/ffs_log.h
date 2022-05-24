#include <stdio.h>
#include <stdarg.h>

#ifndef FFS_LOG_H_
#define FFS_LOG_H_

static void FFS_DBG_INFO(const char* format, ...) {
    // return;

    printf("[INFO] ");
    va_list v_args;
    va_start(v_args, format);
    vprintf(format, v_args);
    va_end(v_args);
}

static void FFS_DBG_ERR(const char* format, ...) {
    return;

    printf("[ERR] ");
    va_list v_args;
    va_start(v_args, format);
    vprintf(format, v_args);
    va_end(v_args);
}

static void FFS_DBG_WARN(const char* format, ...) {
    return;

    printf("[WARN] ");
    va_list v_args;
    va_start(v_args, format);
    vprintf(format, v_args);
    va_end(v_args);
}

#endif