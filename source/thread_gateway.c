#include "thread_gateway.h"
#include "predefined.h"

extern int32_t zoo_get_children(zhandle_t *zh, const char *path, int watch, struct String_vector *strings);
extern int32_t zoo_get(zhandle_t *zh, const char *path, int watch, char *buffer, int *buffer_len, struct Stat *stat);

static struct MHD_Daemon *thread_gateway = NULL;
static struct zhandle_t *thread_gateway_zookeeper = NULL;

static thread_gateway_rule thread_gateway_zookeeper_rule = GATEWAY_ROUNDROBIN;

static void thread_gateway_zookeeper_watcher(
		zhandle_t *handle,
		int32_t type,
		int32_t state,
		const char *path,
		void *watcher)
{
	
}

static int8_t thread_gateway_zookeeper_connect()
{
	zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
	
	thread_gateway_zookeeper = 
		zookeeper_init(
		"127.0.0.1:2181", 
		thread_gateway_zookeeper_watcher, 
		2000, 
		0, 
		0, 
		0);

	if (!thread_gateway_zookeeper)
	{
		DBG_WARN("failed to connect zookeeper service");
		return -1;
	}

	return 0;
}

static char *thread_gateway_zookeeper_balance()
{	
	static struct String_vector modules;

	if (zoo_get_children(thread_gateway_zookeeper, "/modules", 0, &modules) != ZOK)
	{
		DBG_WARN("failed to get modules");
		return "";
	}

	if (modules.count < 0)
	{
		DBG_WARN("failed to get proper module");
		return "";
	}

	static int balance = 0;
	static int balance_size = 256;

	static char balance_module[256];
	static char balance_url[256];

	switch (thread_gateway_zookeeper_rule)
	{
		case GATEWAY_ROUNDROBIN:
		{
			static int32_t roundrobin_turn = 0;
			
			balance = roundrobin_turn++ % modules.count;
			break;
		}

		default:
		{
			DBG_WARN("invalid load balancing rule");
			return "";
		}
	}

	snprintf(
	balance_module,
	sizeof(balance_module),
	"/modules/%s",
	modules.data[balance]);

	if (zoo_get(thread_gateway_zookeeper, balance_module, 0, balance_url, &balance_size, NULL) < 0)
	{
		DBG_WARN("failed to get proper module url");
		return "";
	}
	
	return balance_url;
}

static int32_t thread_gateway_request_handler(
		void *cls, 
		struct MHD_Connection *connection, 
		const char *url, 
		const char *method, 
		const char *version,
		const char *data,
		int64_t *size,
		void **con_cls) 
{
	if (strncmp(method, "GET", 3) != 0)
	{
		return MHD_NO;
	}

	struct MHD_Response *response = MHD_create_response_from_buffer(0, "", MHD_RESPMEM_PERSISTENT);

	if (!response)
	{
		return MHD_NO;
	}
	
	static int ret = -1;
	static char *redirect = NULL;
	
	redirect = thread_gateway_zookeeper_balance();

	if (strlen(redirect) > 0)
	{
		MHD_add_response_header(response, "Location", redirect);
		
		ret = MHD_queue_response(connection, MHD_HTTP_FOUND, response);

		MHD_destroy_response(response);
	}

	return ret;
}

extern void thread_gateway_start()
{	
	thread_gateway = MHD_start_daemon(
			MHD_USE_SELECT_INTERNALLY,
			80,
			NULL,
			NULL,
			&thread_gateway_request_handler,
			NULL,
			MHD_OPTION_END);

	if (!thread_gateway)
	{
		DBG_WARN("failed to start gateway thread");
		return;
	}

	if (thread_gateway_zookeeper_connect() < 0)
	{
		thread_gateway_stop();

		DBG_WARN("failed to start gateway thread");
		return;
	}

	DBG_INFO("gateway thread started");
	return;
}

extern void thread_gateway_stop()
{	
	MHD_stop_daemon(thread_gateway);
	zookeeper_close(thread_gateway_zookeeper);

	DBG_INFO("gateway thread terminated");
	return;
}

extern void thread_gateway_set_rule(thread_gateway_rule rule)
{
	static char notice[256];

	switch (rule)
	{
		case GATEWAY_ROUNDROBIN: 
		{
			rule = GATEWAY_ROUNDROBIN;

			snprintf(
			notice, 
			sizeof(notice), 
			"round-robin load balancing rule selected");
			
			break;
		}

		default:
		{
			DBG_WARN("invalid load balancing rule");
			return;
		}
	}

	DBG_INFO("%s", notice);
	return;
}
