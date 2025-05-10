#include "thread_consumer.h"
#include "thread_producer.h"
#include "predefined.h"

audio_device *g_audio_playback = NULL;
thread_consumer g_thread_consumer = CONSUMER_NONE;

extern void *thread_consumer_playback(void *argument)
{
	if (g_thread_producer == PRODUCER_NONE)
	{
		DBG_WARN("failed to start playback consumer thread");
		return NULL;
	}
	
	if (g_thread_consumer != CONSUMER_NONE)
	{
		DBG_WARN("failed to start playback consumer thread");
		return NULL;
	}

	DBG_INFO("playback consumer thread started");

	g_audio_playback = CREATE(audio_device);

	int16_t raw_buffer[AUD_BUFFER_FRAMES * AUD_CHANNELS];
	int32_t raw_frames = 0;
	int32_t raw_samples = 0;

	if (audio_device_open(g_audio_playback, AUD_MODE_PLAYBACK, AUD_CHANNELS, AUD_SAMPLE_RATE) < 0)
	{
		free(g_audio_playback);

		DBG_WARN("playback consumer thread terminated");
		return NULL;
	}

	g_thread_consumer = CONSUMER_PLAYBACK;

	switch (g_thread_producer)
	{
		case PRODUCER_RAW:
		{	
			raw_samples = AUD_BUFFER_FRAMES * AUD_CHANNELS;

			while (g_thread_consumer)
			{
				audio_queue_pop(g_audio_queue, raw_buffer, &raw_samples);
				audio_device_write_frames(g_audio_playback, raw_buffer, &raw_samples);
			}

			break;
		}

		case PRODUCER_AAC:
		{
			int8_t aac_buffer[AAC_BUFFER_PAYLOADS];
			int32_t aac_payloads = 0;
			
			while (g_thread_consumer)
			{
				codec_queue_pop(g_codec_queue, aac_buffer, &aac_payloads);
				codec_aac_decode(g_codec_aac, raw_buffer, aac_buffer, &raw_samples, &aac_payloads);

				audio_device_write_frames(g_audio_playback, raw_buffer, &raw_samples);
			}

			break;
		}

		case PRODUCER_OPUS:
		{
			int8_t opus_buffer[OPUS_BUFFER_PAYLOADS];
			int32_t opus_payloads = 0;
			
			while (g_thread_consumer)
			{
				codec_queue_pop(g_codec_queue, opus_buffer, &opus_payloads);
				codec_opus_decode(g_codec_opus, raw_buffer, opus_buffer, &raw_frames, &opus_payloads);

				raw_samples = raw_frames * AUD_CHANNELS;

				audio_device_write_frames(g_audio_playback, raw_buffer, &raw_samples);
			}

			break;
		}

		default:
		{
			DBG_WARN("invalid producer thread type");
			break;
		}
	}

	audio_device_close(g_audio_playback);
	free(g_audio_playback);

	DBG_INFO("playback consumer thread terminated");
	return NULL;
}

extern void *thread_consumer_transmission_hls(void *argument)
{
	if (g_thread_producer == PRODUCER_NONE)
	{
		DBG_WARN("failed to start hls transmission consumer thread");
		return NULL;
	}
	
	if (g_thread_consumer != CONSUMER_NONE)
	{
		DBG_WARN("failed to start hls transmission consumer thread");
		return NULL;
	}

	FILE *stream = NULL;

	if (stream_hls_open(&stream, (const char *)argument) < 0)
	{
		DBG_WARN("failed to start hls transmission consumer thread");
		return NULL;
	}

	DBG_INFO("hls transmission consumer thread started");

	int8_t aac_buffer[AAC_BUFFER_PAYLOADS];
	int32_t aac_payloads = 0;
			
	g_thread_consumer = CONSUMER_TRANSMISSION_HLS;

	switch (g_thread_producer)
	{
		case PRODUCER_AAC:
		{
			while (g_thread_consumer)
			{
				codec_queue_pop(g_codec_queue, aac_buffer, &aac_payloads);
				stream_hls_transmission_payloads(&stream, aac_buffer, &aac_payloads);
			}

			break;
		}

		default:
		{
			DBG_WARN("invalid producer thread type");
			break;
		}
	}

	stream_hls_close(&stream, (const char *)argument);
	
	DBG_INFO("hls transmission consumer thread terminated");
	return NULL;
}

extern void *thread_consumer_transmission_dash(void *argument)
{
	if (g_thread_producer == PRODUCER_NONE)
	{
		DBG_WARN("failed to start dash transmission consumer thread");
		return NULL;
	}
	
	if (g_thread_consumer != CONSUMER_NONE)
	{
		DBG_WARN("failed to start dash transmission consumer thread");
		return NULL;
	}

	FILE *stream = NULL;

	if (stream_dash_open(&stream, (const char *)argument) < 0)
	{
		DBG_WARN("failed to start dash transmission consumer thread");
		return NULL;
	}

	DBG_INFO("dash transmission consumer thread started");

	int8_t aac_buffer[AAC_BUFFER_PAYLOADS];
	int32_t aac_payloads = 0;
			
	g_thread_consumer = CONSUMER_TRANSMISSION_DASH;

	switch (g_thread_producer)
	{
		case PRODUCER_AAC:
		{
			while (g_thread_consumer)
			{
				codec_queue_pop(g_codec_queue, aac_buffer, &aac_payloads);
				stream_dash_transmission_payloads(&stream, aac_buffer, &aac_payloads);
			}

			break;
		}

		default:
		{
			DBG_WARN("invalid producer thread type");
			break;
		}
	}

	stream_dash_close(&stream);
	
	DBG_INFO("dash transmission consumer thread terminated");
	return NULL;
}

extern void *thread_consumer_transmission_janus(void *argument)
{
	if (g_thread_producer == PRODUCER_NONE)
	{
		DBG_WARN("failed to start janus transmission consumer thread");
		return NULL;
	}
	
	if (g_thread_consumer != CONSUMER_NONE)
	{
		DBG_WARN("failed to start janus transmission consumer thread");
		return NULL;
	}

	FILE *stream = NULL;

	if (stream_janus_open(&stream, (const char *)argument) < 0)
	{
		DBG_WARN("failed to start janus transmission consumer thread");
		return NULL;
	}

	DBG_INFO("janus transmission consumer thread started");

	int8_t opus_buffer[OPUS_BUFFER_PAYLOADS];
	int32_t opus_payloads = 0;
			
	g_thread_consumer = CONSUMER_TRANSMISSION_JANUS;

	switch (g_thread_producer)
	{
		case PRODUCER_OPUS:
		{
			while (g_thread_consumer)
			{
				codec_queue_pop(g_codec_queue, opus_buffer, &opus_payloads);
				stream_janus_transmission_payloads(&stream, opus_buffer, &opus_payloads);
			}

			break;
		}

		default:
		{
			DBG_WARN("invalid producer thread type");
			break;
		}
	}

	stream_janus_close(&stream);
	
	DBG_INFO("janus transmission consumer thread terminated");
	return NULL;
}

