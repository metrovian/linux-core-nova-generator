#include "thread_producer.h"
#include "thread_consumer.h"
#include "thread_monitor.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal) {
	g_thread_producer = PRODUCER_NONE;
	g_thread_consumer = CONSUMER_NONE;
	thread_monitor_stop();
	DBG_WARN("SIGINT");
	return;
}

int32_t main(int32_t argc, char *argv[]) {
	signal(SIGINT, HANDLE_SIGINT);
	pthread_t pthread_producer;
	pthread_t pthread_consumer;
	if (argc == 1) {
		pthread_create(&pthread_producer, NULL, thread_producer_raw, NULL);
	} else if (argc == 2) {
		if (strncmp(argv[1], "aac", 3) == 0) {
			pthread_create(&pthread_producer, NULL, thread_producer_aac, NULL);
		} else if (strncmp(argv[1], "opus", 4) == 0) {
			pthread_create(&pthread_producer, NULL, thread_producer_opus, NULL);
		} else {
			DBG_WARN("invalid codec");
			return -1;
		}
	} else {
		DBG_WARN("invalid parameters");
		return -1;
	}

	usleep(AUD_BUFFER_TIMES);
	pthread_create(&pthread_consumer, NULL, thread_consumer_playback, NULL);
	thread_monitor_start();
	pthread_join(pthread_producer, NULL);
	pthread_join(pthread_consumer, NULL);
	thread_monitor_stop();
	return 0;
}