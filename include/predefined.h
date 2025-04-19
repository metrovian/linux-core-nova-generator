#pragma once
#include <stdio.h>

#define AUD_DEVICE_CAPTURE	"default"
#define AUD_DEVICE_PLAYBACK	"default"
#define AUD_CHANNELS		2
#define AUD_SAMPLE_RATE		48000
#define AUD_BUFFER_FRAMES	960
#define AUD_FORMAT		SND_PCM_FORMAT_S16_LE
#define AUD_MODE_CAPTURE	SND_PCM_STREAM_CAPTURE
#define AUD_MODE_PLAYBACK	SND_PCM_STREAM_PLAYBACK

#define MAX_Q_CAPACITY_AUDIO	48000
#define MAX_Q_CAPACITY_CODEC	48000

#define OPUS_BUFFER_PAYLOADS	2048
#define OPUS_MODE		OPUS_APPLICATION_AUDIO

#define WARN "\033[31m"
#define INFO "\033[32m"
#define RESET "\033[0m"

#define DBG_WARN(message, ...) fprintf(stderr, WARN "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define DBG_INFO(message, ...) fprintf(stderr, INFO "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define CREATE(structure) (structure *)malloc(sizeof(structure))

