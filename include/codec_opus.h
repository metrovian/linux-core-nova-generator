#pragma once
#include <opus/opus.h>

typedef struct {
	OpusEncoder *encoder;
	OpusDecoder *decoder;
} codec_opus;

extern int8_t codec_opus_open(codec_opus *aucod, int16_t channels, int32_t sample_rate);
extern int8_t codec_opus_close(codec_opus *aucod);
extern int8_t codec_opus_encode(codec_opus *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_frames, int32_t *packet_payloads);
extern int8_t codec_opus_decode(codec_opus *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_frames, int32_t *packet_payloads);
