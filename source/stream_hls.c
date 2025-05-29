#include "stream_hls.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

extern int8_t stream_hls_open(FILE **stream, const char *path) {
	char command_mount[512];
	char command_stream[512];
	char name_ts[64];
	char name_m3u8[64];
	snprintf(
	    command_mount,
	    sizeof(command_mount),
	    "sudo "
	    "mount "
	    "-t tmpfs "
	    "-o size=%dM "
	    "tmpfs "
	    "%s",
	    MAX_M_CAPACITY_TMPFS,
	    path);

	system(command_mount);
	snprintf(name_ts, sizeof(name_ts), "'%s/segment_%%02d.ts'", path);
	snprintf(name_m3u8, sizeof(name_m3u8), "'%s/stream.m3u8'", path);
	snprintf(
	    command_stream,
	    sizeof(command_stream),
	    "sudo "
	    "ffmpeg "
	    "-loglevel error "
	    "-f aac "
	    "-i - "
	    "-c:a copy "
	    "-f hls "
	    "-hls_time 5 "
	    "-hls_list_size 0 "
	    "-hls_segment_filename %s %s",
	    name_ts,
	    name_m3u8);

	*stream = popen(command_stream, "w");
	if (!(*stream)) {
		log_error("failed to open hls stream");
		return -1;
	}

	log_debug("hls stream open success");
	return 0;
}

extern int8_t stream_hls_close(FILE **stream, const char *path) {
	char command_umount[256];
	char command_save[256];
	char name_m3u8[64];
	snprintf(name_m3u8, sizeof(name_m3u8), "'%s/stream.m3u8'", path);
	snprintf(
	    command_save,
	    sizeof(command_save),
	    "sudo "
	    "ffmpeg "
	    "-loglevel error "
	    "-i %s "
	    "-c:a copy "
	    "-bsf:a aac_adtstoasc record_$(date +%%Y%%m%%d_%%H%%M%%S).m4a",
	    name_m3u8);

	snprintf(command_umount, sizeof(command_umount), "sudo umount -f %s", path);
	system(command_save);
	system(command_umount);
	pclose(*stream);
	log_debug("hls stream close success");
	return 0;
}

extern int8_t stream_hls_transmission_payloads(FILE **stream, int8_t *coptr, int32_t *packet_payloads) {
	if (fwrite(coptr, *packet_payloads, 1, *stream) != 1) {
		log_error("failed to stream hls payloads");
		return -1;
	}

	fflush(*stream);
	return 0;
}
