#include "thread_producer.h"
#include "thread_consumer.h"
#include "thread_monitor.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal) {
	g_thread_producer = PRODUCER_NONE;
	g_thread_consumer = CONSUMER_NONE;
	thread_monitor_stop();
	log_warn("SIGINT");
	return;
}

int32_t main(int32_t argc, char *argv[]) {
	signal(SIGINT, HANDLE_SIGINT);
	pthread_t pthread_producer;
	pthread_t pthread_consumer;
	char path_hls[256];
	char path_zk[256];
	char path_kf[256];
	if (argc == 1 || argc == 2) {
		pthread_create(&pthread_producer, NULL, thread_producer_aac, NULL);
		strncpy(path_hls, "/var/www/hls", sizeof(path_hls));
		strncpy(path_zk, "", sizeof(path_zk));
		strncpy(path_kf, "", sizeof(path_kf));
		if (argc == 2) {
			snprintf(path_zk, sizeof(path_zk), "%s:%d", argv[1], NET_ZOOKEEPER_PORT);
			snprintf(path_kf, sizeof(path_kf), "%s:%d", argv[1], NET_KAFKA_PORT);
		}
	} else {
		log_critical("invalid parameters");
		return -1;
	}

	usleep(AUD_BUFFER_TIMES);
	pthread_create(&pthread_consumer, NULL, thread_consumer_transmission_hls, path_hls);
	thread_monitor_resource_ramdisk(path_hls);
	thread_monitor_zookeeper_manager(path_zk);
	thread_monitor_kafka_manager(path_kf);
	usleep(AUD_BUFFER_TIMES);
	thread_monitor_start();
	pthread_join(pthread_producer, NULL);
	pthread_join(pthread_consumer, NULL);
	thread_monitor_stop();
	return 0;
}
