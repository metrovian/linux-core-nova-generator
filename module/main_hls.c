#include "thread_producer.h"
#include "thread_consumer.h"
#include "thread_monitor.h"
#include "wrapper_cxxopts.h"
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
	main_arguments arguments;
	signal(SIGINT, HANDLE_SIGINT);
	if (parse_arguments_hls(&arguments, argc, argv) != 0) {
		log_critical("invalid parameters");
		return -1;
	}

	pthread_t pthread_producer;
	pthread_t pthread_consumer;
	pthread_create(&pthread_producer, NULL, thread_producer_aac, NULL);
	usleep(AUD_BUFFER_TIMES);
	pthread_create(&pthread_consumer, NULL, thread_consumer_transmission_hls, arguments.path_segments);
	thread_monitor_resource_ramdisk(arguments.path_segments);
	thread_monitor_zookeeper_manager(arguments.path_zookeeper);
	thread_monitor_kafka_manager(arguments.path_kafka);
	usleep(AUD_BUFFER_TIMES);
	thread_monitor_start();
	pthread_join(pthread_producer, NULL);
	pthread_join(pthread_consumer, NULL);
	thread_monitor_stop();
	return 0;
}
