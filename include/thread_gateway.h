#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <zookeeper/zookeeper.h>

typedef enum
{
	GATEWAY_ROUNDROBIN = 0,
	GATEWAY_LEASTUSER = 1,
	GATEWAY_LEASTCPU = 2,
	GATEWAY_LEASTNETWORK = 3,
} thread_gateway_rule;

extern void thread_gateway_start();
extern void thread_gateway_stop();

extern void thread_gateway_set_rule(thread_gateway_rule rule);

