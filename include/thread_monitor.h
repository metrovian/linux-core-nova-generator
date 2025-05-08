#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

extern void thread_monitor_audio(int16_t *auptr, int32_t *read_samples);
extern void thread_monitor_start();
extern void thread_monitor_stop();

extern void *thread_monitor(void *argument);
