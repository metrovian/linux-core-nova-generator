#include "thread_monitor.h"
#include "wrapper_spdlog.h"
#include "predefined.h"
#include "preshared.h"

extern int32_t zoo_create(
    zhandle_t *zh,
    const char *path,
    const char *value,
    int value_len,
    const struct ACL_vector *acl,
    int flags,
    char *path_buffer,
    int path_buffer_len);

extern int32_t zoo_set(
    zhandle_t *zh,
    const char *path,
    const char *buffer,
    int buffer_len,
    int version);

static pthread_mutex_t thread_monitor_audio_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t thread_monitor_codec_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t thread_monitor_stream_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct timespec thread_monitor_clock_start;
static struct timespec thread_monitor_clock_end;
static char thread_monitor_resource_path[256];
static char thread_monitor_zookeeper_path[256];
static char thread_monitor_kafka_path[256];
static int8_t thread_monitor_run = 0;
static int32_t thread_monitor_audio_volume = 0;
static int32_t thread_monitor_audio_count = 0;
static int32_t thread_monitor_codec_bitrate = 0;
static int32_t thread_monitor_codec_count = 0;
static int32_t thread_monitor_stream_bitrate = 0;
static int32_t thread_monitor_stream_count = 0;

static void thread_monitor_zookeeper_watcher(
    zhandle_t *handle,
    int32_t type,
    int32_t state,
    const char *path,
    void *watcher) {
	if (state == ZOO_CONNECTED_STATE) {
		return;
	}

	log_critical("invalid zookeeper state");
	return;
}

static void thread_monitor_kafka_watcher(
    rd_kafka_t *handle,
    const rd_kafka_message_t *message,
    void *opaque) {
	if (message->err) {
		log_error("failed to deliver: %s\n", rd_kafka_message_errstr(message));
		return;
	}

	return;
}

static char *thread_monitor_get_ipv4() {
	int32_t fd_socket;
	static char ipv4[INET_ADDRSTRLEN];
	static char iface[INET_ADDRSTRLEN] = NET_INTERFACE;
	fd_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_socket == -1) {
		log_error("failed to get ipv4 address");
		return "";
	}

	struct ifreq ifr;
	strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	if (ioctl(fd_socket, SIOCGIFADDR, &ifr) == -1) {
		log_error("failed to get ipv4 address");
		return "";
	}

	struct sockaddr_in *ipaddr = (struct sockaddr_in *)(&ifr.ifr_addr);
	inet_ntop(AF_INET, &ipaddr->sin_addr, ipv4, INET_ADDRSTRLEN);
	close(fd_socket);
	return ipv4;
}

extern void thread_monitor_audio_capture(int16_t *auptr, int32_t *read_samples) {
	double square = 0;
	double rms = 0;
	for (int32_t i = 0; i < *read_samples; ++i) {
		square += (double)(auptr[i] * auptr[i]);
	}

	rms = sqrt(square / (double)(*read_samples));
	rms = (rms > 0.5) ? rms : 0.5;
	pthread_mutex_lock(&thread_monitor_audio_mutex);
	thread_monitor_audio_volume += (int32_t)(20 * log10(rms / 32768.0));
	thread_monitor_audio_count += (int32_t)1;
	pthread_mutex_unlock(&thread_monitor_audio_mutex);
	return;
}

extern void thread_monitor_codec_encode(int32_t *packet_payloads) {
	pthread_mutex_lock(&thread_monitor_codec_mutex);
	thread_monitor_codec_bitrate += *packet_payloads * 8;
	thread_monitor_codec_count += 1;
	pthread_mutex_unlock(&thread_monitor_codec_mutex);
	return;
}

extern void thread_monitor_stream_consume(int32_t *packet_payloads) {
	pthread_mutex_lock(&thread_monitor_stream_mutex);
	thread_monitor_stream_bitrate += *packet_payloads * 8;
	thread_monitor_stream_count += 1;
	pthread_mutex_unlock(&thread_monitor_stream_mutex);
	return;
}

extern void thread_monitor_resource_ramdisk(const char *path) {
	strncpy(thread_monitor_resource_path, path, sizeof(thread_monitor_resource_path));
	return;
}

extern void thread_monitor_zookeeper_gateway(const char *path) {
	strncpy(thread_monitor_zookeeper_path, path, sizeof(thread_monitor_zookeeper_path));
	return;
}

extern void thread_monitor_kafka_gateway(const char *path) {
	strncpy(thread_monitor_kafka_path, path, sizeof(thread_monitor_kafka_path));
	return;
}

extern void thread_monitor_start() {
	thread_monitor_run = 1;
	pthread_t monitor;
	pthread_create(&monitor, NULL, thread_monitor, NULL);
	pthread_detach(monitor);
	return;
}

extern void thread_monitor_stop() {
	thread_monitor_run = 0;
	return;
}

extern void *thread_monitor(void *argument) {
	FILE *stream_cpu = NULL;
	FILE *stream_memory = NULL;
	FILE *stream_network = NULL;
	zhandle_t *zookeeper_handle = NULL;
	rd_kafka_t *kafka_handle = NULL;
	int32_t audio_volume = 0;
	int32_t codec_bitrate = 0;
	int32_t stream_bitrate = 0;
	int32_t zookeeper_code = 0;
	time_t time_interval = 0;
	time_t time_interval_sec = 0;
	time_t time_interval_nsec = 0;
	char command_cpu[512];
	char command_memory[512];
	char command_network[512];
	char resource_cpu[32] = "0";
	char resource_memory[32] = "0";
	char resource_network[32] = "0";
	char resource_json[2048] = "0";
	char kafka_error[512] = "0";
	snprintf(
	    command_cpu,
	    sizeof(command_cpu),
	    "mpstat 1 %d | "
	    "grep 'Average' | "
	    "awk '{print int(100-$12)}' | "
	    "tr -d '\n'",
	    SYS_MONITOR_INTERVALS / 3000);

	if (strlen(thread_monitor_resource_path)) {
		snprintf(
		    command_memory,
		    sizeof(command_memory),
		    "df -h | "
		    "grep tmpfs | "
		    "grep %s | "
		    "awk '{print $5}' | "
		    "tr -d '%%\n'",
		    thread_monitor_resource_path);
	} else {
		snprintf(
		    command_memory,
		    sizeof(command_memory),
		    "top -bn1 | "
		    "grep 'MiB Mem' | "
		    "awk '{print int((int($8)*100)/int($4))}' | "
		    "tr -d '\n'");
	}

	snprintf(
	    command_network,
	    sizeof(command_network),
	    "ifstat -i %s %d 1 | "
	    "awk 'NR==3 {print $2}'",
	    NET_INTERFACE,
	    SYS_MONITOR_INTERVALS / 3000);

	if (strlen(thread_monitor_zookeeper_path) > 0) {
		zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
		zookeeper_handle =
		    zookeeper_init(
			thread_monitor_zookeeper_path,
			thread_monitor_zookeeper_watcher,
			NET_ZOOKEEPER_TIMEOUT,
			0,
			0,
			0);

		if (!zookeeper_handle) {
			log_error("failed to connect zookeeper service");
			return NULL;
		}

		log_info("zookeeper service started");
	}

	if (strlen(thread_monitor_kafka_path) > 0) {
		rd_kafka_conf_t *kafka_conf = rd_kafka_conf_new();
		rd_kafka_conf_set_dr_msg_cb(kafka_conf, thread_monitor_kafka_watcher);
		rd_kafka_conf_set(
		    kafka_conf,
		    "log_level",
		    "0",
		    kafka_error,
		    sizeof(kafka_error));

		if (rd_kafka_conf_set(
			kafka_conf,
			"bootstrap.servers",
			thread_monitor_kafka_path,
			kafka_error,
			sizeof(kafka_error) != RD_KAFKA_CONF_OK)) {
			log_error("failed to connnect kafka service");
			return NULL;
		}

		kafka_handle =
		    rd_kafka_new(
			RD_KAFKA_PRODUCER,
			kafka_conf,
			kafka_error,
			sizeof(kafka_error));

		if (!kafka_handle) {
			log_error("failed to create kafka producer");
		}

		struct rd_kafka_metadata *kafka_metadata = NULL;
		rd_kafka_resp_err_t kafka_connect = rd_kafka_metadata(kafka_handle, 0, NULL, &kafka_metadata, NET_KAFKA_TIMEOUT);
		if (kafka_connect != RD_KAFKA_RESP_ERR_NO_ERROR) {
			rd_kafka_destroy(kafka_handle);
			kafka_handle = NULL;
			log_error("failed to connect kafka producer");
			return NULL;
		}

		log_info("kafka service started");
	}

	log_info("monitor thread started");
	while (thread_monitor_run) {
		clock_gettime(CLOCK_MONOTONIC, &thread_monitor_clock_start);
		stream_cpu = popen(command_cpu, "r");
		stream_memory = popen(command_memory, "r");
		stream_network = popen(command_network, "r");
		if (!stream_cpu) {
			log_error("failed to open cpu stream");
			break;
		}

		if (!stream_memory) {
			log_error("failed to open memory stream");
			break;
		}

		fgets(resource_cpu, sizeof(resource_cpu), stream_cpu);
		fgets(resource_memory, sizeof(resource_memory), stream_memory);
		fgets(resource_network, sizeof(resource_network), stream_network);
		pclose(stream_cpu);
		pclose(stream_memory);
		pclose(stream_network);
		while (thread_monitor_run) {
			usleep(SYS_MONITOR_TIMES);
			clock_gettime(CLOCK_MONOTONIC, &thread_monitor_clock_end);
			time_interval_sec = thread_monitor_clock_end.tv_sec - thread_monitor_clock_start.tv_sec;
			time_interval_nsec = thread_monitor_clock_end.tv_nsec - thread_monitor_clock_start.tv_nsec;
			time_interval = time_interval_sec * 1000 + time_interval_nsec / 1000000;
			if (time_interval > SYS_MONITOR_INTERVALS) {
				break;
			}
		}

		pthread_mutex_lock(&thread_monitor_audio_mutex);
		if (thread_monitor_audio_count) {
			audio_volume = thread_monitor_audio_volume / thread_monitor_audio_count;
		}

		thread_monitor_audio_volume = 0;
		thread_monitor_audio_count = 0;
		pthread_mutex_unlock(&thread_monitor_audio_mutex);
		pthread_mutex_lock(&thread_monitor_codec_mutex);
		if (thread_monitor_codec_count) {
			codec_bitrate = thread_monitor_codec_bitrate / time_interval;
		}

		thread_monitor_codec_bitrate = 0;
		thread_monitor_codec_count = 0;
		pthread_mutex_unlock(&thread_monitor_codec_mutex);
		pthread_mutex_lock(&thread_monitor_stream_mutex);
		if (thread_monitor_stream_count) {
			stream_bitrate = thread_monitor_stream_bitrate / time_interval;
		}

		thread_monitor_stream_bitrate = 0;
		thread_monitor_stream_count = 0;
		pthread_mutex_unlock(&thread_monitor_stream_mutex);
		if (zookeeper_handle) {
			static thread_monitor_data zookeeper_data = {
			    .name = "",
			    .url = "",
			    .user = 0,
			    .cpu = 0,
			    .network = 0,
			};

			snprintf(
			    zookeeper_data.url,
			    sizeof(zookeeper_data.url),
			    "http://%s",
			    thread_monitor_get_ipv4());

			zookeeper_data.cpu = atoi(resource_cpu);
			zookeeper_data.network = atoi(resource_network);
			zookeeper_code = 0;
			if (strlen(zookeeper_data.name) == 0) {
				zookeeper_code =
				    zoo_create(
					zookeeper_handle,
					NET_ZOOKEEPER_NODE,
					NULL,
					-1,
					&ZOO_OPEN_ACL_UNSAFE,
					0,
					NULL,
					0);

				zookeeper_code =
				    zoo_create(
					zookeeper_handle,
					NET_ZOOKEEPER_NAME,
					(char *)(&zookeeper_data),
					sizeof(zookeeper_data),
					&ZOO_OPEN_ACL_UNSAFE,
					ZOO_EPHEMERAL | ZOO_SEQUENCE,
					zookeeper_data.name,
					sizeof(zookeeper_data.name));

				if (zookeeper_code != ZOK) {
					log_error("failed to create zookeeper node: %d", zookeeper_code);
					return NULL;
				}
			}

			zookeeper_code =
			    zoo_set(
				zookeeper_handle,
				zookeeper_data.name,
				(char *)(&zookeeper_data),
				sizeof(zookeeper_data),
				-1);

			if (zookeeper_code != ZOK) {
				log_error("failed to refresh zookeeper node: %d", zookeeper_code);
				return NULL;
			}

			if (kafka_handle) {
				snprintf(
				    resource_json,
				    sizeof(resource_json),
				    "nova_resource_cpu{url=\"%s\"} %s\n"
				    "nova_resource_mem{url=\"%s\"} %s\n"
				    "nova_resource_rec{url=\"%s\"} %d\n"
				    "nova_resource_enc{url=\"%s\"} %d\n"
				    "nova_resource_str{url=\"%s\"} %d\n",
				    zookeeper_data.url, resource_cpu,
				    zookeeper_data.url, resource_memory,
				    zookeeper_data.url, audio_volume,
				    zookeeper_data.url, codec_bitrate,
				    zookeeper_data.url, stream_bitrate);

				rd_kafka_resp_err_t kafka_resp =
				    rd_kafka_producev(
					kafka_handle,
					RD_KAFKA_V_TOPIC(NET_KAFKA_TOPIC),
					RD_KAFKA_V_VALUE(resource_json, strlen(resource_json)),
					RD_KAFKA_V_END);

				if (kafka_resp != RD_KAFKA_RESP_ERR_NO_ERROR) {
					log_error("failed to produce kafka stream");
					return NULL;
				}

				rd_kafka_poll(kafka_handle, 0);
			}
		}

		log_info(
		    "cpu: %-3s%% | "
		    "mem: %-3s%% | "
		    "rec: %-3d dbfs | "
		    "enc: %-3d kbps | "
		    "str: %-3d kbps | ",
		    resource_cpu,
		    resource_memory,
		    audio_volume,
		    codec_bitrate,
		    stream_bitrate);
	}

	if (strlen(thread_monitor_zookeeper_path) > 0) {
		zookeeper_close(zookeeper_handle);
		zookeeper_handle = NULL;
		log_info("zookeeper service terminated");
	}

	if (strlen(thread_monitor_kafka_path) > 0) {
		rd_kafka_flush(kafka_handle, NET_KAFKA_TIMEOUT);
		rd_kafka_destroy(kafka_handle);
		kafka_handle = NULL;
		log_info("kafka service terminated");
	}

	log_info("monitor thread terminated");
	return NULL;
}
