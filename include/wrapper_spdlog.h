#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
extern void log_trace(const char *format, ...);
extern void log_debug(const char *format, ...);
extern void log_info(const char *format, ...);
extern void log_warn(const char *format, ...);
extern void log_error(const char *format, ...);
extern void log_critical(const char *format, ...);
#ifdef __cplusplus
}
#endif