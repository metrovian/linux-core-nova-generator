#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
	int8_t *data;
	int32_t *packets;
	int32_t capacity;
	int32_t size;
	int32_t front;
	int32_t back;
	int32_t size_packets;
	int32_t front_packets;
	int32_t back_packets;
	pthread_mutex_t mutex;
	pthread_cond_t push_available;
	pthread_cond_t pop_available;
} codec_queue;

extern codec_queue *codec_queue_create(int32_t cocap);
extern void codec_queue_destroy(codec_queue *coque);
extern void codec_queue_push(codec_queue *coque, int8_t *coptr, int32_t *push_payloads);
extern void codec_queue_pop(codec_queue *coque, int8_t *coptr, int32_t *pop_payloads);
