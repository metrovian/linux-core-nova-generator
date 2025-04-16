#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct
{
	int16_t *data;
	int32_t capacity;
	int32_t size;
	int32_t front;
	int32_t back;
	pthread_mutex_t mutex;
	pthread_cond_t push_available;
	pthread_cond_t pop_available;
} audio_queue;

extern audio_queue *audio_queue_create(int32_t aucap);
extern void audio_queue_destroy(audio_queue *auque);
extern void audio_queue_push(audio_queue *auque, int16_t *auptr, int32_t *push_frames);
extern void audio_queue_pop(audio_queue *auque, int16_t *auptr, int32_t *pop_frames);

