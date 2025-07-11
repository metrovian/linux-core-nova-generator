#pragma once
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
	char path_segments[256];
	char path_zookeeper[256];
	char path_kafka[256];
} main_arguments;

extern int8_t parse_arguments_hls(main_arguments *args, int32_t argc, char *argv[]);
extern int8_t parse_arguments_dash(main_arguments *args, int32_t argc, char *argv[]);
#ifdef __cplusplus
}
#endif