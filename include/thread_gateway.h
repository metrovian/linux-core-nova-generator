#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <microhttpd.h>
#include <zookeeper/zookeeper.h>
#include <librdkafka/rdkafka.h>
#include <curl/curl.h>
#include <curl/curlver.h>
#include <curl/easy.h>
#include <curl/urlapi.h>

typedef enum
{
	GATEWAY_ROUNDROBIN = 0,
	GATEWAY_LEASTCPU = 1,
	GATEWAY_LEASTNETWORK = 2,
} thread_gateway_rule;

extern void thread_gateway_start();
extern void thread_gateway_stop();

extern void thread_gateway_set_rule(thread_gateway_rule rule);

