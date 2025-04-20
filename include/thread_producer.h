#pragma once
#include "audio_device.h"
#include "audio_queue.h"
#include "codec_opus.h"
#include "codec_queue.h"

typedef enum
{
	PRODUCER_NONE = 0,
	PRODUCER_RAW = 1,
	PRODUCER_OPUS = 2,
} thread_producer;

extern audio_device *g_audio_capture;
extern audio_queue *g_audio_queue;
extern codec_opus *g_codec_opus;
extern codec_queue *g_codec_queue;
extern thread_producer g_thread_producer;

extern void *thread_producer_raw(void *argument);
extern void *thread_producer_opus(void *argument);
