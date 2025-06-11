#include "thread_manager.h"
#include "wrapper_spdlog.h"
#include "predefined.h"
#include "preshared.h"

extern int32_t zoo_get_children(
    zhandle_t *zh,
    const char *path,
    int watch,
    struct String_vector *strings);

extern int32_t zoo_get(
    zhandle_t *zh,
    const char *path,
    int watch,
    char *buffer,
    int *buffer_len,
    struct Stat *stat);

extern int32_t zoo_wget(
    zhandle_t *zh,
    const char *path,
    watcher_fn watcher,
    void *watcher_ctx,
    char *buffer,
    int *buffer_len,
    struct Stat *stat);

static struct MHD_Daemon *thread_manager = NULL;
static struct zhandle_t *thread_manager_zookeeper = NULL;
static struct rd_kafka_t *thread_manager_kafka = NULL;
static struct String_vector thread_manager_zookeeper_modules;
static thread_manager_rule thread_manager_zookeeper_rule = MANAGER_ROUNDROBIN;
static thread_manager_data thread_manager_zookeeper_data[32];
static int32_t thread_manager_round_robin = 0;
static int32_t thread_manager_least_user = 0;
static int32_t thread_manager_least_cpu = 0;
static int32_t thread_manager_least_network = 0;

static void thread_manager_zookeeper_watcher(zhandle_t *handle, int32_t type, int32_t state, const char *path, void *watcher) {
	if (state == ZOO_CONNECTED_STATE) {
		static int modules_now = 0;
		static int modules_prev = 0;
		static int module_size = sizeof(thread_manager_data);
		static char module_name[256];
		if (type == ZOO_CHILD_EVENT) {
			zoo_get_children(
			    handle,
			    path,
			    1,
			    &thread_manager_zookeeper_modules);

			if (thread_manager_zookeeper_modules.count != modules_now) {
				modules_prev = modules_now;
				modules_now = thread_manager_zookeeper_modules.count;
				for (int32_t i = 0; i < modules_now; ++i) {
					snprintf(
					    module_name,
					    sizeof(module_name),
					    "%s/%s",
					    NET_ZOOKEEPER_NODE,
					    thread_manager_zookeeper_modules.data[i]);

					zoo_wget(
					    handle,
					    module_name,
					    thread_manager_zookeeper_watcher,
					    NULL,
					    (char *)(&thread_manager_zookeeper_data[i]),
					    &module_size,
					    NULL);
				}

				if (modules_now > modules_prev) {
					log_info("zookeeper module connected: %d", modules_now);
					return;
				} else {
					log_info("zookeeper module disconnected: %d", modules_now);
					return;
				}
			}
		}

		if (type == ZOO_CHANGED_EVENT) {
			int32_t event_index = 0;
			for (event_index = 0; event_index < modules_now; ++event_index) {
				if (strncmp(
					thread_manager_zookeeper_data[event_index].name,
					path,
					sizeof(thread_manager_zookeeper_data[event_index].name)) == 0) {
					break;
				}
			}

			zoo_wget(
			    handle,
			    path,
			    thread_manager_zookeeper_watcher,
			    NULL,
			    (char *)(&thread_manager_zookeeper_data[event_index]),
			    &module_size,
			    NULL);

			return;
		}

		if (type == ZOO_SESSION_EVENT) {
			return;
		}

		if (type == ZOO_CREATED_EVENT) {
			return;
		}

		if (type == ZOO_DELETED_EVENT) {
			return;
		}

		log_critical("invalid zookeeper event: %d", type);
		return;
	}

	log_critical("invalid zookeeper state: %d", state);
	return;
}

static int8_t thread_manager_zookeeper_connect() {
	zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
	thread_manager_zookeeper =
	    zookeeper_init(
		NET_ZOOKEEPER_LOCAL,
		thread_manager_zookeeper_watcher,
		NET_ZOOKEEPER_TIMEOUT,
		NULL,
		NULL,
		0);

	if (!thread_manager_zookeeper) {
		log_error("failed to connect zookeeper service");
		return -1;
	}

	zoo_get_children(thread_manager_zookeeper, NET_ZOOKEEPER_NODE, 1, NULL);
	log_info("zookeeper service started");
	return 0;
}

static char *thread_manager_zookeeper_balance() {
	if (thread_manager_zookeeper_modules.count < 0) {
		log_error("failed to get proper module");
		return "";
	}

	static int balance = 0;
	int32_t user = INT32_MAX;
	int32_t cpu = INT32_MAX;
	int32_t network = INT32_MAX;
	for (int32_t i = 0; i < thread_manager_zookeeper_modules.count; ++i) {
		if (user > thread_manager_zookeeper_data[i].user) {
			user = thread_manager_zookeeper_data[i].user;
			thread_manager_least_user = i;
		}

		if (cpu > thread_manager_zookeeper_data[i].cpu) {
			cpu = thread_manager_zookeeper_data[i].cpu;
			thread_manager_least_cpu = i;
		}

		if (network > thread_manager_zookeeper_data[i].network) {
			network = thread_manager_zookeeper_data[i].network;
			thread_manager_least_network = i;
		}
	}

	switch (thread_manager_zookeeper_rule) {
	case MANAGER_ROUNDROBIN: {
		balance = thread_manager_round_robin++ % thread_manager_zookeeper_modules.count;
		break;
	}

	case MANAGER_LEASTCPU: {
		balance = thread_manager_least_cpu;
		break;
	}

	case MANAGER_LEASTNETWORK: {
		balance = thread_manager_least_network;
		break;
	}

	default: {
		log_critical("invalid load balancing rule");
		return "";
	}
	}

	return thread_manager_zookeeper_data[balance].url;
}

static int8_t thread_manager_kafka_connect() {
	rd_kafka_conf_t *kafka_conf = rd_kafka_conf_new();
	rd_kafka_topic_partition_list_t *kafka_topics = rd_kafka_topic_partition_list_new(1);
	char kafka_error[512];

	rd_kafka_conf_set(
	    kafka_conf,
	    "log_level",
	    "0",
	    kafka_error,
	    sizeof(kafka_error));

	rd_kafka_conf_set(
	    kafka_conf,
	    "bootstrap.servers",
	    NET_KAFKA_LOCAL,
	    kafka_error,
	    sizeof(kafka_error));

	rd_kafka_conf_set(
	    kafka_conf,
	    "group.id",
	    "resource-monitor",
	    kafka_error,
	    sizeof(kafka_error));

	thread_manager_kafka = rd_kafka_new(
	    RD_KAFKA_CONSUMER,
	    kafka_conf,
	    kafka_error,
	    sizeof(kafka_error));

	if (!thread_manager_kafka) {
		log_error("failed to create kafka consumer");
		return -1;
	}

	struct rd_kafka_metadata *kafka_metadata = NULL;
	rd_kafka_resp_err_t kafka_connect = rd_kafka_metadata(thread_manager_kafka, 0, NULL, &kafka_metadata, NET_KAFKA_TIMEOUT);
	if (kafka_connect != RD_KAFKA_RESP_ERR_NO_ERROR) {
		rd_kafka_destroy(thread_manager_kafka);
		thread_manager_kafka = NULL;
		log_error("failed to connect kafka consumer");
		return -1;
	}

	rd_kafka_poll_set_consumer(thread_manager_kafka);
	rd_kafka_topic_partition_list_add(kafka_topics, NET_KAFKA_TOPIC, -1);
	rd_kafka_subscribe(thread_manager_kafka, kafka_topics);
	log_info("kafka service started");
	return 0;
}

static int32_t thread_manager_request_handler(
    void *cls,
    struct MHD_Connection *connection,
    const char *url,
    const char *method,
    const char *version,
    const char *data,
    int64_t *size,
    void **con_cls) {
	if (strncmp(method, "GET", 3) != 0) {
		return MHD_NO;
	}

	struct MHD_Response *response = MHD_create_response_from_buffer(0, "", NET_MANAGER_RESPONSE);
	if (!response) {
		return MHD_NO;
	}

	static int ret = -1;
	static char *redirect = NULL;
	redirect = thread_manager_zookeeper_balance();
	if (strlen(redirect) > 0) {
		MHD_add_response_header(response, "Location", redirect);
		ret = MHD_queue_response(connection, MHD_HTTP_FOUND, response);
		MHD_destroy_response(response);
	}

	return ret;
}

extern void thread_manager_start() {
	thread_manager =
	    MHD_start_daemon(
		NET_MANAGER_MODE,
		NET_MANAGER_PORT,
		NULL,
		NULL,
		&thread_manager_request_handler,
		NULL,
		MHD_OPTION_END);

	if (thread_manager) {
		log_info("redirect service started");
	} else {

		log_error("failed to start manager thread");
		return;
	}

	if (thread_manager_zookeeper_connect() < 0) {
		thread_manager_stop();
		log_error("failed to start manager thread");
		return;
	}

	if (thread_manager_kafka_connect() < 0) {
		thread_manager_stop();
		log_error("failed to start manager thread");
		return;
	}

	log_info("manager thread started");
	while (thread_manager_kafka) {
		rd_kafka_message_t *kafka_message = rd_kafka_consumer_poll(thread_manager_kafka, NET_KAFKA_TIMEOUT);
		if (kafka_message) {
			if (kafka_message->err) {
				rd_kafka_message_destroy(kafka_message);
				log_error("failed to consume kafka stream");
				continue;
			}

			if (kafka_message->len > 0) {
				CURL *curl = curl_easy_init();
				curl_easy_setopt(curl, CURLOPT_URL, NET_KAFKA_PUSHGATE);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, kafka_message->payload);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, kafka_message->len);
				curl_easy_perform(curl);
				curl_easy_cleanup(curl);
			}

			rd_kafka_message_destroy(kafka_message);
		}
	}

	log_info("manager thread terminated");
	return;
}

extern void thread_manager_stop() {
	if (thread_manager_zookeeper) {
		zookeeper_close(thread_manager_zookeeper);
		thread_manager_zookeeper = NULL;
		log_info("zookeeper service terminated");
	}

	if (thread_manager_kafka) {
		rd_kafka_flush(thread_manager_kafka, NET_KAFKA_TIMEOUT);
		rd_kafka_destroy(thread_manager_kafka);
		thread_manager_kafka = NULL;
		log_info("kafka service terminated");
	}

	if (thread_manager) {
		MHD_stop_daemon(thread_manager);
		thread_manager = NULL;
		log_info("redirect service terminated");
	}

	return;
}

extern void thread_manager_set_rule(thread_manager_rule rule) {
	static char notice[256];
	switch (rule) {
	case MANAGER_ROUNDROBIN: {
		thread_manager_zookeeper_rule = MANAGER_ROUNDROBIN;
		snprintf(
		    notice,
		    sizeof(notice),
		    "round-robin load balancer selected");

		break;
	}

	case MANAGER_LEASTCPU: {
		thread_manager_zookeeper_rule = MANAGER_LEASTCPU;
		snprintf(
		    notice,
		    sizeof(notice),
		    "least-cpu load balancer selected");

		break;
	}

	case MANAGER_LEASTNETWORK: {
		thread_manager_zookeeper_rule = MANAGER_LEASTNETWORK;
		snprintf(
		    notice,
		    sizeof(notice),
		    "least-network load balancer selected");

		break;
	}

	default: {
		log_error("invalid load balancing rule");
		return;
	}
	}

	log_info("%s", notice);
	return;
}
