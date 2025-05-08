#include "thread_monitor.h"
#include "predefined.h"

static pthread_mutex_t thread_monitor_audio_mutex = PTHREAD_MUTEX_INITIALIZER;

static int8_t thread_monitor_run = 0;
static int32_t thread_monitor_audio_volume = 0;
static int32_t thread_monitor_audio_count = 0;

extern void thread_monitor_audio(int16_t *auptr, int32_t *read_samples)
{
	double square = 0;
	double rms = 0;

	for (int32_t i = 0; i < *read_samples; ++i)
	{
		square += (double)(auptr[i] * auptr[i]);
	}

	rms = sqrt(square / (double)(*read_samples));
	
	pthread_mutex_lock(&thread_monitor_audio_mutex);

	thread_monitor_audio_volume += (int32_t)(20 * log10(rms / 32768.0));
	thread_monitor_audio_count += (int32_t)1;

	pthread_mutex_unlock(&thread_monitor_audio_mutex);

	return;
}

extern void thread_monitor_start()
{
	thread_monitor_run = 1;

	pthread_t monitor;
	pthread_create(&monitor, NULL, thread_monitor, NULL);	
	pthread_detach(monitor);

	return;
}

extern void thread_monitor_stop()
{
	thread_monitor_run = 0;

	return;
}

extern void *thread_monitor(void *argument)
{
	int32_t audio_volume = 0;

	DBG_INFO("monitor thread started");

	while (thread_monitor_run)
	{
		pthread_mutex_lock(&thread_monitor_audio_mutex);
		
		audio_volume = thread_monitor_audio_volume / thread_monitor_audio_count;
		thread_monitor_audio_volume = 0;
		thread_monitor_audio_count = 0;

		pthread_mutex_unlock(&thread_monitor_audio_mutex);

		DBG_INFO("%d dBFS", audio_volume);

		usleep(1000000);
	}

	DBG_INFO("monitor thread terminated");
	return NULL;
}
