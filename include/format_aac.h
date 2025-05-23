#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern int8_t format_aac_read_file(const char *name, int8_t **coptr, int16_t *channels, int32_t *sample_rate, int32_t *read_payloads);
extern int8_t format_aac_write_file(const char *name, int8_t **coptr, int16_t *channels, int32_t *sample_rate, int32_t *write_payloads);

