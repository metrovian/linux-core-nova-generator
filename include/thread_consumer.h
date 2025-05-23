#pragma once
#include "audio_device.h"
#include "audio_queue.h"
#include "codec_aac.h"
#include "codec_opus.h"
#include "codec_queue.h"
#include "stream_hls.h"
#include "stream_dash.h"
#include "stream_janus.h"

typedef enum {
	CONSUMER_NONE = 0,
	CONSUMER_PLAYBACK = 1,
	CONSUMER_TRANSMISSION_HLS = 2,
	CONSUMER_TRANSMISSION_DASH = 3,
	CONSUMER_TRANSMISSION_JANUS = 4,
} thread_consumer;

extern audio_device *g_audio_playback;
extern thread_consumer g_thread_consumer;

extern void *thread_consumer_playback(void *argument);
extern void *thread_consumer_transmission_hls(void *argument);
extern void *thread_consumer_transmission_dash(void *argument);
extern void *thread_consumer_transmission_janus(void *argument);

