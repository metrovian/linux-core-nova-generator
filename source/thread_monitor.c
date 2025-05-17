#include "thread_monitor.h"
#include "predefined.h"

static pthread_mutex_t thread_monitor_audio_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t thread_monitor_codec_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t thread_monitor_stream_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct timespec thread_monitor_clock_start;
static struct timespec thread_monitor_clock_end;

static char thread_monitor_resource_path[256];
static char thread_monitor_zookeeper_path[256];

static int8_t thread_monitor_run = 0;
static int32_t thread_monitor_audio_volume = 0;
static int32_t thread_monitor_audio_count = 0;
static int32_t thread_monitor_codec_bitrate = 0;
static int32_t thread_monitor_codec_count = 0;
static int32_t thread_monitor_stream_bitrate = 0;
static int32_t thread_monitor_stream_count = 0;

static void thread_monitor_zookeeper_watcher(
                zhandle_t *handle,
                int32_t type,
                int32_t state,
                const char *path,
                void *watcher)
{
        if (state == ZOO_CONNECTED_STATE)
        {
                DBG_INFO("zookeeper service connected");
                return;
        }

        DBG_WARN("invalid zookeeper state");
        return;
}

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

extern void thread_monitor_zookeeper_gateway(const char *path)
{
	strncpy(thread_monitor_zookeeper_path, path, sizeof(thread_monitor_zookeeper_path));

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

	zhandle_t *zookeeper_handle = NULL;

	snprintf(
	command_cpu,
	sizeof(command_cpu),
	"mpstat 1 %d | "
	"grep 'Average' | "
	"awk '{print int(100-$12)}' | "
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

	if (strlen(thread_monitor_zookeeper_path) > 0)
	{
		zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);

		zookeeper_handle =
			zookeeper_init(
			thread_monitor_zookeeper_path,
			thread_monitor_zookeeper_watcher,
			3000,
			0,
			0,
			0);

		if (!zookeeper_handle)
		{
			DBG_WARN("failed to connect zookeeper service");
			return NULL;
		}
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

		if (zookeeper_handle)
		{
			static char zookeeper_path[256] = "";
			static char zookeeper_url[256] = "http://192.168.50.100";
			
			int32_t zookeeper_code = 0;

			if (strlen(zookeeper_path) == 0)
			{
				zookeeper_code =
					zoo_create(
					zookeeper_handle,
					"/modules",
					NULL,
					-1,
					&ZOO_OPEN_ACL_UNSAFE,
					0,
					NULL,
					0);

				zookeeper_code = 
					zoo_create(
					zookeeper_handle,
					"/modules/module-",
					zookeeper_url,
					strlen(zookeeper_url),
					&ZOO_OPEN_ACL_UNSAFE,
					ZOO_EPHEMERAL |
					ZOO_SEQUENCE,
					zookeeper_path,
					sizeof(zookeeper_path));
			}

			else
			{
				zookeeper_code = 
					zoo_create(
					zookeeper_handle,
					zookeeper_path,
					zookeeper_url,
					strlen(zookeeper_url),
					&ZOO_OPEN_ACL_UNSAFE,
					ZOO_EPHEMERAL,
					NULL,
					-1);
			}

			if (zookeeper_code == ZNODEEXISTS)
			{
				zookeeper_code =
					zoo_set(
					zookeeper_handle,
					zookeeper_path,
					zookeeper_url,
					strlen(zookeeper_url),
					-1);
			}

			else if (zookeeper_code != ZOK)
			{
				DBG_WARN("failed to create zookeeper node : %d", zookeeper_code);
				return NULL;
			}
		}

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
