#include "codec_queue.h"
#include "thread_monitor.h"
#include "predefined.h"

extern codec_queue *codec_queue_create(int32_t cocap)
{
	codec_queue *queue = (codec_queue *)malloc(sizeof(codec_queue));

	if (!queue)
	{
		DBG_WARN("failed to allocate codec queue");
		return NULL;
	}

	queue->data = (int8_t *)malloc(sizeof(int8_t) * cocap);

	if (!queue->data)
	{
		free(queue);

		DBG_WARN("failed to allocate codec queue data");
		return NULL;
	}

	queue->packets = (int32_t *)malloc(sizeof(int32_t) * cocap);

	if (!queue->packets)
	{
		free(queue->data);
		free(queue);

		DBG_WARN("failed to allocate codec queue frames");
		return NULL;
	}

	queue->capacity = cocap;
	queue->size = 0;
	queue->size_packets = 0;

	queue->front = 0;
	queue->back = 0;
	queue->front_packets = 0;
	queue->back_packets = 0;

	pthread_mutex_init(&queue->mutex, NULL);

	pthread_cond_init(&queue->push_available, NULL);
	pthread_cond_init(&queue->pop_available, NULL);

	return queue;
}

extern void codec_queue_destroy(codec_queue *coque)
{
	if (!coque)
	{
		return;
	}

	pthread_mutex_destroy(&coque->mutex);

	pthread_cond_destroy(&coque->push_available);
	pthread_cond_destroy(&coque->pop_available);

	free(coque->packets);
	free(coque->data);
	free(coque);

	return;
}

extern void codec_queue_push(codec_queue *coque, int8_t *coptr, int32_t *push_payloads)
{
	pthread_mutex_lock(&coque->mutex);

	while (coque->capacity < coque->size + *push_payloads)
	{
		pthread_cond_wait(&coque->push_available, &coque->mutex);
	}
	
	for (int32_t i = 0; i < *push_payloads; ++i)
	{
		coque->data[coque->back] = coptr[i];

		coque->back = (coque->back + 1) % coque->capacity;
		coque->size = (coque->size + 1);
	}
	
	coque->packets[coque->back_packets] = *push_payloads;

	coque->back_packets = (coque->back_packets + 1) % coque->capacity;
	coque->size_packets = (coque->size_packets + 1);
	
	pthread_cond_signal(&coque->pop_available);
	pthread_mutex_unlock(&coque->mutex);

	thread_monitor_codec_encode(push_payloads);
	return;
}

extern void codec_queue_pop(codec_queue *coque, int8_t *coptr, int32_t *pop_payloads)
{
	pthread_mutex_lock(&coque->mutex);

	while (coque->size_packets < 1)
	{
		pthread_cond_wait(&coque->pop_available, &coque->mutex);
	}

	*pop_payloads = coque->packets[coque->front_packets];

	coque->front_packets = (coque->front_packets + 1) % coque->capacity;
	coque->size_packets = (coque->size_packets - 1);

	for (int32_t i = 0; i < *pop_payloads; ++i)
	{
		coptr[i] = coque->data[coque->front];

		coque->front = (coque->front + 1) % coque->capacity;
		coque->size = (coque->size - 1);
	}
	
	pthread_cond_signal(&coque->push_available);
	pthread_mutex_unlock(&coque->mutex);

	thread_monitor_stream_consume(pop_payloads);
	return;
}

