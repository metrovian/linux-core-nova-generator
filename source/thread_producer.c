#include "thread_producer.h"
#include "predefined.h"

audio_device *g_audio_capture;
audio_queue *g_audio_queue;
codec_opus *g_codec_opus;
codec_queue *g_codec_queue;

int8_t g_run_producer_raw = 0;
int8_t g_run_producer_opus = 0;

extern void *thread_producer_raw(void *argument)
{
	DBG_INFO("raw producer thread started");
	
	g_audio_capture = CREATE(audio_device);

	int16_t raw_buffer[AUD_BUFFER_FRAMES * AUD_CHANNELS];
	int32_t raw_samples = 0;

	if (audio_device_open(g_audio_capture, AUD_MODE_CAPTURE, AUD_CHANNELS, AUD_SAMPLE_RATE) < 0)
	{
		free(g_audio_capture);

		DBG_WARN("raw producer thread terminated");
		return NULL;
	}

	g_run_producer_raw = 1;
	g_audio_queue = audio_queue_create(MAX_Q_CAPACITY_AUDIO);

	if (!g_audio_queue)
	{
		audio_device_close(g_audio_capture);
		free(g_audio_capture);
		
		DBG_WARN("raw producer thread terminated");
		return NULL;
	}
	
	while (g_run_producer_raw)
	{
		if (audio_device_read_frames(g_audio_capture, raw_buffer, &raw_samples) == 0) 
		{
			audio_queue_push(g_audio_queue, raw_buffer, &raw_samples);
		}
	}

	audio_device_close(g_audio_capture);
	free(g_audio_capture);

	DBG_INFO("raw producer thread terminated");
	return NULL;
}

extern void *thread_producer_opus(void *argument)
{
	DBG_INFO("opus producer thread started");	

	g_audio_capture = CREATE(audio_device);
	g_codec_opus = CREATE(codec_opus);

	int16_t raw_buffer[AUD_BUFFER_FRAMES * AUD_CHANNELS];
	int32_t raw_frames = 0;
	int32_t raw_samples = 0;

	int8_t opus_buffer[OPUS_BUFFER_PAYLOADS];
	int32_t opus_payloads = 0;

	if (audio_device_open(g_audio_capture, AUD_MODE_CAPTURE, AUD_CHANNELS, AUD_SAMPLE_RATE) < 0)
	{
		free(g_audio_capture);
		free(g_codec_opus);

		DBG_WARN("opus producer thread terminated");
		return NULL;
	}
	
	if (codec_opus_open(g_codec_opus, AUD_CHANNELS, AUD_SAMPLE_RATE) < 0)
	{
		audio_device_close(g_audio_capture);
		free(g_audio_capture);
		free(g_codec_opus);

		DBG_WARN("opus producer thread terminated");
		return NULL;
	}
	
	g_run_producer_opus = 1;
	g_codec_queue = codec_queue_create(MAX_Q_CAPACITY_CODEC);

	if (!g_codec_queue)
	{
		audio_device_close(g_audio_capture);
		codec_opus_close(g_codec_opus);
		free(g_audio_capture);
		free(g_codec_opus);

		DBG_WARN("opus producer thread terminated");
		return NULL;
	}

	while (g_run_producer_opus)
	{
		if (audio_device_read_frames(g_audio_capture, raw_buffer, &raw_samples) == 0) 
		{
			raw_frames = raw_samples / AUD_CHANNELS;

			if (codec_opus_encode(g_codec_opus, raw_buffer, opus_buffer, &raw_frames, &opus_payloads) == 0)
			{
				codec_queue_push(g_codec_queue, opus_buffer, &opus_payloads);
			}
		}
	}

	audio_device_close(g_audio_capture);
	codec_opus_close(g_codec_opus);
	free(g_audio_capture);
	free(g_codec_opus);

	DBG_INFO("opus producer thread terminated");
	return NULL;
}
