#include "thread_producer.h"
#include "thread_consumer.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal) {
	g_thread_producer = PRODUCER_NONE;
	g_thread_consumer = CONSUMER_NONE;
	log_warn("SIGINT");
	return;
}

void HANDLE_SIGPIPE(int32_t signal) {
	log_warn("SIGPIPE");
	return;
}

int32_t main(int32_t argc, char *argv[]) {
	signal(SIGINT, HANDLE_SIGINT);
	signal(SIGPIPE, HANDLE_SIGPIPE);
	pthread_t pthread_producer;
	pthread_t pthread_consumer;
	if (argc == 2) {
		pthread_create(&pthread_producer, NULL, thread_producer_opus, NULL);
		usleep(AUD_BUFFER_TIMES);
		pthread_create(&pthread_consumer, NULL, thread_consumer_transmission_janus, argv[1]);
	} else {
		log_critical("invalid parameters");
		return -1;
	}

	pthread_join(pthread_producer, NULL);
	pthread_join(pthread_consumer, NULL);
	return 0;
}
