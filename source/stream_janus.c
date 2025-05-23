#include "stream_janus.h"
#include "predefined.h"

extern int8_t stream_janus_open(FILE **stream, const char *path) {
	char command_stream[512];
	snprintf(
	    command_stream,
	    sizeof(command_stream),
	    "sudo "
	    "gst-launch-1.0 -q "
	    "fdsrc ! "
	    "queue ! "
	    "identity sync=true ! "
	    "opusparse ! "
	    "rtpopuspay ! "
	    "udpsink host=%s port=5555",
	    path);

	*stream = popen(command_stream, "w");
	if (!(*stream)) {
		DBG_WARN("failed to open janus stream");
		return -1;
	}

	DBG_INFO("janus stream open success");
	return 0;
}

extern int8_t stream_janus_close(FILE **stream) {
	pclose(*stream);
	DBG_INFO("janus stream close success");
	return 0;
}

extern int8_t stream_janus_transmission_payloads(FILE **stream, int8_t *coptr, int32_t *packet_payloads) {
	if (fwrite(coptr, *packet_payloads, 1, *stream) != 1) {
		DBG_WARN("failed to stream payloads");
		return -1;
	}

	fflush(*stream);
	return 0;
}
