#pragma once
#include <stdio.h>

#define AUD_DEVICE_CAPTURE	"default"
#define AUD_DEVICE_PLAYBACK	"default"

#define AUD_CHANNELS		1
#define AUD_SAMPLE_RATE		44100
#define AUD_BUFFER_FRAMES	44100

#define AUD_FORMAT		SND_PCM_FORMAT_S16_LE
#define AUD_MODE_CAPTURE	SND_PCM_STREAM_CAPTURE
#define AUD_MODE_PLAYBACK	SND_PCM_STREAM_PLAYBACK

#define WARN "\033[31m"
#define INFO "\033[32m"
#define RESET "\033[0m"

#define DBG_WARN(message, ...) fprintf(stderr, WARN "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define DBG_INFO(message, ...) fprintf(stderr, INFO "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)
