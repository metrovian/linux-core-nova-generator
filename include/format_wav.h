#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
	int8_t riff_id[4];
	int32_t riff_size;
	int8_t wave_id[4];
	int8_t fmt_id[4];
	int32_t fmt_size;
	int16_t fmt_type;
	int16_t channels;
	int32_t sample_rate;
	int32_t byte_rate;
	int16_t block_align;
	int16_t bits_per_sample;
	int8_t data_id[4];
	int32_t data_size;
} header_wav;
#pragma pack(pop)

extern int8_t format_wav_read_file(const char *name, int16_t **auptr, int16_t *channels, int32_t *sample_rate, int32_t *read_frames);
extern int8_t format_wav_write_file(const char *name, int16_t **auptr, int16_t *channels, int32_t *sample_rate, int32_t *write_frames);
