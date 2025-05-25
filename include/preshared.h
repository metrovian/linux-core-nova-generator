#pragma once
#pragma pack(push, 1)
typedef struct {
	char name[256];
	char url[256];
	int32_t user;
	int32_t cpu;
	int32_t network;
} thread_gateway_data, thread_monitor_data;
#pragma pack(pop)