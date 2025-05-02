#include "stream_dash.h"
#include "predefined.h"

extern int8_t stream_dash_open(FILE **stream, const char *path)
{
	char stream_command[512];
	char name_mpd[64];
	
	snprintf(name_mpd, sizeof(name_mpd), "'%s/stream.mpd'", path);
	
	snprintf(
	stream_command, 
	sizeof(stream_command),
	"sudo "
	"ffmpeg "
	"-loglevel error "
	"-f aac "
	"-i - "
	"-c:a copy "
	"-f dash "
	"-segment_time 5 "
	"-y %s", 
	name_mpd);

	*stream = popen(stream_command, "w");

	if(!*stream)
	{
		DBG_WARN("failed to open dash stream");
		return -1;
	}

	DBG_INFO("dash stream open success");
	return 0;
}

extern int8_t stream_dash_close(FILE **stream)
{
	pclose(*stream);

	DBG_INFO("dash stream close success");
	return 0;
}

extern int8_t stream_dash_transmission_payloads(FILE **stream, int8_t *coptr, int32_t *packet_payloads)
{
	if (fwrite(coptr, *packet_payloads, 1, *stream) != 1)
	{
		DBG_WARN("failed to stream payloads");
		return -1;
	}

	fflush(*stream);

	return 0;
}
