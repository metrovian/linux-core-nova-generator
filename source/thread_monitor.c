#include "thread_monitor.h"
#include "predefined.h"

static pthread_mutex_t thread_monitor_audio_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t thread_monitor_codec_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t thread_monitor_stream_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct timespec thread_monitor_clock_start;
static struct timespec thread_monitor_clock_end;

static char thread_monitor_resource_path[256];

static int8_t thread_monitor_run = 0;
static int32_t thread_monitor_audio_volume = 0;
static int32_t thread_monitor_audio_count = 0;
static int32_t thread_monitor_codec_bitrate = 0;
static int32_t thread_monitor_codec_count = 0;
static int32_t thread_monitor_stream_bitrate = 0;
static int32_t thread_monitor_stream_count = 0;

extern void thread_monitor_audio_capture(int16_t *auptr, int32_t *read_samples)
{
	double square = 0;
	double rms = 0;

	for (int32_t i = 0; i < *read_samples; ++i)
	{
		square += (double)(auptr[i] * auptr[i]);
	}

	rms = sqrt(square / (double)(*read_samples));
	rms = (rms > 0.5) ? rms : 0.5;

	pthread_mutex_lock(&thread_monitor_audio_mutex);

	thread_monitor_audio_volume += (int32_t)(20 * log10(rms / 32768.0));
	thread_monitor_audio_count += (int32_t)1;

	pthread_mutex_unlock(&thread_monitor_audio_mutex);

	return;
}

extern void thread_monitor_codec_encode(int32_t *packet_payloads)
{
	pthread_mutex_lock(&thread_monitor_codec_mutex);

	thread_monitor_codec_bitrate += *packet_payloads * 8;	
	thread_monitor_codec_count += 1;

	pthread_mutex_unlock(&thread_monitor_codec_mutex);

	return;
}

extern void thread_monitor_stream_consume(int32_t *packet_payloads)
{
	pthread_mutex_lock(&thread_monitor_stream_mutex);

	thread_monitor_stream_bitrate += *packet_payloads * 8;
	thread_monitor_stream_count += 1;

	pthread_mutex_unlock(&thread_monitor_stream_mutex);

	return;
}

extern void thread_monitor_resource_ramdisk(const char *path)
{
	strncpy(thread_monitor_resource_path, path, sizeof(thread_monitor_resource_path));

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
	FILE *stream_cpu = NULL;
	FILE *stream_memory = NULL;

	char command_cpu[512];
	char command_memory[512];

	char resource_cpu[32];
	char resource_memory[32];

	int32_t audio_volume = 0;
	int32_t codec_bitrate = 0;
	int32_t stream_bitrate = 0;

	time_t time_interval = 0;
	time_t time_interval_sec = 0;
	time_t time_interval_nsec = 0;	
	
	snprintf(
	command_cpu,
	sizeof(command_cpu),
	"top -bn%d -d1 | "
	"grep '%%Cpu(s)' | "
	"awk '{print (100-$8)}' | "
	"awk '{sum+=$1} END {print int(sum/NR)}' | "
	"tr -d '\n'",
	SYS_MONITOR_INTERVALS / 1000);

	if (strlen(thread_monitor_resource_path))
	{
		snprintf(
		command_memory,
		sizeof(command_memory),
		"df -h | "
		"grep tmpfs | "
		"grep %s | "
		"awk '{print $5}' | "
		"tr -d '%%\n'",
		thread_monitor_resource_path);
	}

	else
	{
		snprintf(
		command_memory,
		sizeof(command_memory),
		"top -bn1 | "
		"grep 'MiB Mem' | "
		"awk '{print int((int($8)*100)/int($4))}' | "
		"tr -d '\n'");	
	}

	DBG_INFO("monitor thread started");

	while (thread_monitor_run)
	{	
		clock_gettime(CLOCK_MONOTONIC, &thread_monitor_clock_start);
		
		stream_cpu = popen(command_cpu, "r");
		stream_memory = popen(command_memory, "r");
		
		if (!stream_cpu)
		{
			DBG_WARN("failed to open cpu stream");
			break;
		}

		if (!stream_memory)
		{
			DBG_WARN("failed to open memory stream");
			break;
		}
		
		fgets(resource_cpu, sizeof(resource_cpu), stream_cpu);
		fgets(resource_memory, sizeof(resource_memory), stream_memory);

		pclose(stream_cpu);
		pclose(stream_memory);
	
		while (thread_monitor_run)
		{
			usleep(SYS_MONITOR_TIMES);

			clock_gettime(CLOCK_MONOTONIC, &thread_monitor_clock_end);
			
			time_interval_sec = thread_monitor_clock_end.tv_sec - thread_monitor_clock_start.tv_sec;
			time_interval_nsec = thread_monitor_clock_end.tv_nsec - thread_monitor_clock_start.tv_nsec;

			time_interval = time_interval_sec * 1000 + time_interval_nsec / 1000000;
			
			if (time_interval > SYS_MONITOR_INTERVALS)
			{
				break;
			}
		}
	
		pthread_mutex_lock(&thread_monitor_audio_mutex);
		
		if (thread_monitor_audio_count)
		{
			audio_volume = thread_monitor_audio_volume / thread_monitor_audio_count;
		}

		thread_monitor_audio_volume = 0;
		thread_monitor_audio_count = 0;

		pthread_mutex_unlock(&thread_monitor_audio_mutex);
		pthread_mutex_lock(&thread_monitor_codec_mutex);

		if (thread_monitor_codec_count)
		{
			codec_bitrate = thread_monitor_codec_bitrate / time_interval;
		}

		thread_monitor_codec_bitrate = 0;
		thread_monitor_codec_count = 0;

		pthread_mutex_unlock(&thread_monitor_codec_mutex);
		pthread_mutex_lock(&thread_monitor_stream_mutex);

		if (thread_monitor_stream_count)
		{
			stream_bitrate = thread_monitor_stream_bitrate / time_interval;
		}

		thread_monitor_stream_bitrate = 0;
		thread_monitor_stream_count = 0;

		pthread_mutex_unlock(&thread_monitor_stream_mutex);

		DBG_INFO(
		"cpu: %-3s%% | "
		"mem: %-3s%% | "
		"rec: %-3d dbfs | "
		"enc: %-3d kbps | "
		"str: %-3d kbps | ",
	       	resource_cpu,
		resource_memory,
		audio_volume, 
		codec_bitrate,
		stream_bitrate);
	}

	DBG_INFO("monitor thread terminated");
	return NULL;
}
