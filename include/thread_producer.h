#pragma once
#include "audio_device.h"
#include "audio_queue.h"
#include "codec_opus.h"
#include "codec_queue.h"

extern audio_device *g_audio_capture;
extern audio_queue *g_audio_queue;
extern codec_opus *g_codec_opus;
extern codec_queue *g_codec_queue;

extern int8_t g_run_producer_raw;
extern int8_t g_run_producer_opus;

extern void *thread_producer_raw(void *argument);
extern void *thread_producer_opus(void *argument);
