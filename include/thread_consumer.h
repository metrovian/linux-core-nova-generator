#include "audio_device.h"
#include "audio_queue.h"
#include "codec_opus.h"
#include "codec_queue.h"

typedef enum
{
	CONSUMER_NONE = 0,
	CONSUMER_PLAYBACK = 1,
	CONSUMER_RECORD = 2,
	CONSUMER_TRANSMISSION = 3,
} thread_consumer;

extern audio_device *g_audio_playback;
extern thread_consumer g_thread_consumer;

extern void *thread_consumer_playback(void *argument);
extern void *thread_consumer_record(void *argument);
extern void *thread_consumer_transmission(void *argument);
