// clang-format off
#pragma once
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define AUD_DEVICE_CAPTURE	"default"
#define AUD_DEVICE_PLAYBACK	"default"
#define AUD_CHANNELS		2
#define AUD_SAMPLE_RATE		48000
#define AUD_BUFFER_FRAMES	1024
#define AUD_BUFFER_TIMES	500000
#define AUD_ACCESS              SND_PCM_ACCESS_RW_INTERLEAVED
#define AUD_FORMAT		SND_PCM_FORMAT_S16_LE
#define AUD_MODE_CAPTURE	SND_PCM_STREAM_CAPTURE
#define AUD_MODE_PLAYBACK	SND_PCM_STREAM_PLAYBACK

#define NET_INTERFACE		"enp0s3"
#define NET_ZOOKEEPER_LOCAL	"localhost:2181"
#define NET_ZOOKEEPER_NODE	"/modules"
#define NET_ZOOKEEPER_NAME	"/modules/module-"
#define NET_ZOOKEEPER_PORT	2181
#define NET_ZOOKEEPER_TIMEOUT	3000

#define NET_KAFKA_LOCAL		"localhost:9094"
#define NET_KAFKA_PUSHGATE	"localhost:9095/metrics/job/resource"
#define NET_KAFKA_TOPIC		"resource"
#define NET_KAFKA_PORT		9092
#define NET_KAFKA_TIMEOUT	3000

#define NET_MANAGER_PORT	8888
#define NET_MANAGER_MODE	MHD_USE_SELECT_INTERNALLY
#define NET_MANAGER_RESPONSE	MHD_RESPMEM_PERSISTENT

#define SYS_LOGGER_SYSTEM       "system"
#define SYS_LOGGER_FORMAT       "log"
#define SYS_LOGGER_SIZE         4096
#define SYS_LOGGER_LOGLEVEL     2

#define SYS_MONITOR_INTERVALS	3000
#define SYS_MONITOR_TIMES	1000

#define MAX_M_CAPACITY_TMPFS	1024
#define MAX_Q_CAPACITY_AUDIO	480000
#define MAX_Q_CAPACITY_CODEC	480000
#define MAX_Q_WAIT_AUDIO        1
#define MAX_Q_WAIT_CODEC        1

#define AAC_BIT_RATE		128000
#define AAC_BUFFER_FRAMES	1024
#define AAC_BUFFER_PAYLOADS	2048
#define AAC_FORMAT		AUDIO_FORMAT_PCM_16_BIT
#define AAC_TRANSMUX		TT_MP4_ADTS
#define AAC_MODE		AOT_AAC_LC

#define OPUS_BUFFER_FRAMES	960
#define OPUS_BUFFER_PAYLOADS	2048
#define OPUS_MODE		OPUS_APPLICATION_AUDIO

#define WARN "\033[31m"
#define INFO "\033[32m"
#define RESET "\033[0m"

#define DBG_WARN(message, ...) fprintf(stderr, WARN "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define DBG_INFO(message, ...) fprintf(stderr, INFO "[%s:%d] " RESET message "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define CREATE(structure) (structure *)malloc(sizeof(structure))
// clang-format on