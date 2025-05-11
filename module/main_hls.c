#include "thread_producer.h"
#include "thread_consumer.h"
#include "thread_monitor.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal)
{
	g_thread_producer = PRODUCER_NONE;
	g_thread_consumer = CONSUMER_NONE;
	
	thread_monitor_stop();

	DBG_WARN("SIGINT");
	return;
}

int32_t main(int32_t argc, char *argv[])
{	
	signal(SIGINT, HANDLE_SIGINT);

	pthread_t pthread_producer;
	pthread_t pthread_consumer;

	char path_hls[256];

	if (argc == 1 || argc == 2)
	{
		pthread_create(&pthread_producer, NULL, thread_producer_aac, NULL);
		
		if (argc == 1) strncpy(path_hls, "/var/www/hls", sizeof(path_hls));
		if (argc == 2) strncpy(path_hls, argv[1], sizeof(path_hls));
	}

	else
	{
		DBG_WARN("invalid parameters");
		return -1;
	}

	usleep(AUD_BUFFER_TIMES);
	
	pthread_create(&pthread_consumer, NULL, thread_consumer_transmission_hls, path_hls);

	thread_monitor_resource_ramdisk(path_hls);
	thread_monitor_start();
	
	while (g_thread_producer);

	pthread_join(pthread_producer, NULL);
	pthread_join(pthread_consumer, NULL);	
	
	thread_monitor_stop();

	return 0;
}

