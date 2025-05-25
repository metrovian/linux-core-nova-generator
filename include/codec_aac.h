#pragma once
#include <fdk-aac/aacenc_lib.h>
#include <fdk-aac/aacdecoder_lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
	HANDLE_AACENCODER encoder;
	HANDLE_AACDECODER decoder;
} codec_aac;

extern int8_t codec_aac_open(codec_aac *aucod, int16_t channels, int32_t sample_rate);
extern int8_t codec_aac_close(codec_aac *aucod);
extern int8_t codec_aac_encode(codec_aac *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_samples, int32_t *packet_payloads);
extern int8_t codec_aac_decode(codec_aac *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_samples, int32_t *packet_payloads);
