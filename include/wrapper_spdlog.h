#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
	LOG_TRACE = 0,
	LOG_DEBUG = 1,
	LOG_INFO = 2,
	LOG_WARN = 3,
	LOG_ERROR = 4,
	LOG_CRITICAL = 5,
} wrapper_spdlog_level;

extern void wrapper_spdlog_trace(const char *msg);
extern void wrapper_spdlog_debug(const char *msg);
extern void wrapper_spdlog_info(const char *msg);
extern void wrapper_spdlog_warn(const char *msg);
extern void wrapper_spdlog_error(const char *msg);
extern void wrapper_spdlog_critical(const char *msg);
#ifdef __cplusplus
}
#endif