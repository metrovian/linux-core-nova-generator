#include "thread_producer.h"
#include "thread_consumer.h"
#include "predefined.h"

void HANDLE_SIGINT(int32_t signal)
{
	g_thread_producer = PRODUCER_NONE;
	g_thread_consumer = CONSUMER_NONE;
	
	DBG_WARN("SIGINT");
	return;
}

int32_t main(int32_t argc, char *argv[])
{	
	signal(SIGINT, HANDLE_SIGINT);

	pthread_t pthread_producer;
	pthread_t pthread_consumer;

	if (argc == 2)
	{
		pthread_create(&pthread_producer, NULL, thread_producer_aac, NULL);

		usleep(AUD_BUFFER_TIMES);

		pthread_create(&pthread_consumer, NULL, thread_consumer_transmission_dash, argv[1]);
	}

	else
	{
		DBG_WARN("invalid parameters");
		return -1;
	}

	while (g_thread_producer);

	pthread_join(pthread_producer, NULL);
	pthread_join(pthread_consumer, NULL);	
	
	return 0;
}

