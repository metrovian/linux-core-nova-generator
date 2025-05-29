#include "audio_queue.h"
#include "thread_monitor.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

extern audio_queue *audio_queue_create(int32_t aucap) {
	audio_queue *queue = (audio_queue *)malloc(sizeof(audio_queue));
	if (!queue) {
		log_error("failed to allocate audio queue structure");
		return NULL;
	}

	queue->data = (int16_t *)malloc(sizeof(int16_t) * aucap);
	if (!queue->data) {
		free(queue);
		log_error("failed to allocate audio queue data");
		return NULL;
	}

	queue->capacity = aucap;
	queue->size = 0;
	queue->front = 0;
	queue->back = 0;
	pthread_mutex_init(&queue->mutex, NULL);
	pthread_condattr_init(&queue->attribute);
	pthread_condattr_setclock(&queue->attribute, CLOCK_MONOTONIC);
	pthread_cond_init(&queue->push_available, &queue->attribute);
	pthread_cond_init(&queue->pop_available, &queue->attribute);
	log_debug("audio queue create success");
	return queue;
}

extern void audio_queue_destroy(audio_queue *auque) {
	if (auque) {
		pthread_mutex_destroy(&auque->mutex);
		pthread_cond_destroy(&auque->push_available);
		pthread_cond_destroy(&auque->pop_available);
		free(auque->data);
		free(auque);
		log_debug("audio queue destroy success");
	}

	return;
}

extern void audio_queue_push(audio_queue *auque, int16_t *auptr, int32_t *push_samples) {
	pthread_mutex_lock(&auque->mutex);
	while (auque->capacity < auque->size + *push_samples) {
		struct timespec timeout;
		clock_gettime(CLOCK_MONOTONIC, &timeout);
		timeout.tv_sec += MAX_Q_WAIT_AUDIO;
		if (pthread_cond_timedwait(&auque->push_available, &auque->mutex, &timeout) == ETIMEDOUT) {
			log_error("failed to push audio frames without overwrite");
			break;
		}
	}

	for (int32_t i = 0; i < *push_samples; ++i) {
		auque->data[auque->back] = auptr[i];
		auque->back = (auque->back + 1) % auque->capacity;
		auque->size = (auque->size + 1);
	}

	pthread_cond_signal(&auque->pop_available);
	pthread_mutex_unlock(&auque->mutex);
	return;
}

extern void audio_queue_pop(audio_queue *auque, int16_t *auptr, int32_t *pop_samples) {
	pthread_mutex_lock(&auque->mutex);
	while (auque->size < *pop_samples) {
		struct timespec timeout;
		clock_gettime(CLOCK_MONOTONIC, &timeout);
		timeout.tv_sec += MAX_Q_WAIT_AUDIO;
		if (pthread_cond_timedwait(&auque->pop_available, &auque->mutex, &timeout) == ETIMEDOUT) {
			log_error("failed to pop audio frames within the expected time");
			break;
		}
	}

	for (int32_t i = 0; i < *pop_samples; ++i) {
		auptr[i] = auque->data[auque->front];
		auque->front = (auque->front + 1) % auque->capacity;
		auque->size = (auque->size - 1);
	}

	pthread_cond_signal(&auque->push_available);
	pthread_mutex_unlock(&auque->mutex);
	thread_monitor_stream_consume(pop_samples);
	return;
}
