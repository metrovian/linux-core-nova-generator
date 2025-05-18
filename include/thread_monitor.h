#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <zookeeper/zookeeper.h>

extern void thread_monitor_audio_capture(int16_t *auptr, int32_t *read_samples);
extern void thread_monitor_codec_encode(int32_t *packet_payloads);
extern void thread_monitor_stream_consume(int32_t *packet_payloads);

extern void thread_monitor_resource_ramdisk(const char *path);
extern void thread_monitor_zookeeper_gateway(const char *path);

extern void thread_monitor_start();
extern void thread_monitor_stop();

extern void *thread_monitor(void *argument);
