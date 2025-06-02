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

typedef enum {
	MANAGER_ROUNDROBIN = 0,
	MANAGER_LEASTCPU = 1,
	MANAGER_LEASTNETWORK = 2,
} thread_manager_rule;

extern void thread_manager_start();
extern void thread_manager_stop();
extern void thread_manager_set_rule(thread_manager_rule rule);
