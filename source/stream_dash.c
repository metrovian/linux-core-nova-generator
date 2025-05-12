#include "stream_dash.h"
#include "predefined.h"

extern int8_t stream_dash_open(FILE **stream, const char *path)
{
	char command_mount[512];
	char command_stream[512];
	char name_mpd[64];
	
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

	snprintf(name_mpd, sizeof(name_mpd), "'%s/stream.mpd'", path);

	snprintf(
	command_stream, 
	sizeof(command_stream),
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

	*stream = popen(command_stream, "w");

	if(!*stream)
	{
		DBG_WARN("failed to open dash stream");
		return -1;
	}

	DBG_INFO("dash stream open success");
	return 0;
}

extern int8_t stream_dash_close(FILE **stream, const char *path)
{
	char command_umount[256];
	char command_save[256];
	char name_mpd[64];
	
	snprintf(name_mpd, sizeof(name_mpd), "'%s/stream.mpd'", path);

	snprintf(
	command_save,
	sizeof(command_save),
	"sudo "
	"ffmpeg "
	"-loglevel error "
	"-i %s "
	"-c:a copy record_$(date +%%Y%%m%%d_%%H%%M%%S).m4a",
	name_mpd);

	snprintf(command_umount, sizeof(command_umount), "sudo umount -f %s", path);
	
	system(command_save);
	system(command_umount);

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
