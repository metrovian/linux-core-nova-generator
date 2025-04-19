#include "audio_queue.h"
#include "predefined.h"

extern audio_queue *audio_queue_create(int32_t aucap)
{
	audio_queue *queue = (audio_queue *)malloc(sizeof(audio_queue));

	if (!queue)
	{
		DBG_WARN("failed to allocate audio queue");
		return NULL;
	}

	queue->data = (int16_t *)malloc(sizeof(int16_t) * aucap);

	if (!queue->data)
	{
		free(queue);

		DBG_WARN("failed to allocate audio queue data");
		return NULL;
	}

	queue->capacity = aucap;
	queue->size = 0;

	queue->front = 0;
	queue->back = 0;

	pthread_mutex_init(&queue->mutex, NULL);

	pthread_cond_init(&queue->push_available, NULL);
	pthread_cond_init(&queue->pop_available, NULL);

	return queue;
}

extern void audio_queue_destroy(audio_queue *auque)
{
	if (!auque)
	{
		return;
	}

	pthread_mutex_destroy(&auque->mutex);

	pthread_cond_destroy(&auque->push_available);
	pthread_cond_destroy(&auque->pop_available);

	free(auque->data);
	free(auque);

	return;
}

extern void audio_queue_push(audio_queue *auque, int16_t *auptr, int32_t *push_samples)
{
	pthread_mutex_lock(&auque->mutex);

	while (auque->capacity < auque->size + *push_samples)
	{
		pthread_cond_wait(&auque->push_available, &auque->mutex);
	}
	
	for (int32_t i = 0; i < *push_samples; ++i)
	{
		auque->data[auque->back] = auptr[i];

		auque->back = (auque->back + 1) % auque->capacity;
		auque->size = (auque->size + 1);
	}
		
	pthread_cond_signal(&auque->pop_available);
	pthread_mutex_unlock(&auque->mutex);

	return;
}

extern void audio_queue_pop(audio_queue *auque, int16_t *auptr, int32_t *pop_samples)
{
	pthread_mutex_lock(&auque->mutex);

	while (auque->size < *pop_samples)
	{
		pthread_cond_wait(&auque->pop_available, &auque->mutex);
	}

	for (int32_t i = 0; i < *pop_samples; ++i)
	{
		auptr[i] = auque->data[auque->front];

		auque->front = (auque->front + 1) % auque->capacity;
		auque->size = (auque->size - 1);
	}
	
	pthread_cond_signal(&auque->push_available);
	pthread_mutex_unlock(&auque->mutex);

	return;
}

