#pragma once
#include <stdio.h>

#define WARN "\033[31m"
#define INFO "\033[32m"
#define RESET "\033[0m"

#define DBG_WARN(message, ...) fprintf(stderr, WARN "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define DBG_INFO(message, ...) fprintf(stderr, INFO "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)
