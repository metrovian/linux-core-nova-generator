#include "stream_hls.h"
#include "predefined.h"

extern int8_t stream_hls_open(FILE **stream, const char *path)
{
	char mount_command[512];
	char stream_command[512];
	char name_ts[64];
	char name_m3u8[64];
	
	snprintf(
	mount_command,
	sizeof(mount_command),
	"sudo "
	"mount "
	"-t tmpfs "
	"-o size=%dM "
	"tmpfs "
	"%s",
	MAX_M_CAPACITY_TMPFS,
	path);

	system(mount_command);

	snprintf(name_ts, sizeof(name_ts), "'%s/segment_%%02d.ts'", path);
	snprintf(name_m3u8, sizeof(name_m3u8), "'%s/stream.m3u8'", path);

	snprintf(
	stream_command, 
	sizeof(stream_command),
	"sudo "
	"ffmpeg "
	"-loglevel error "
	"-f aac "
	"-i - "
	"-c:a copy "
	"-f hls "
	"-hls_time 5 "
	"-hls_list_size 5 "
	"-hls_segment_filename %s %s",
	name_ts, 
	name_m3u8);

	*stream = popen(stream_command, "w");

	if(!*stream)
	{
		DBG_WARN("failed to open hls stream");
		return -1;
	}

	DBG_INFO("hls stream open success");
	return 0;
}

extern int8_t stream_hls_close(FILE **stream, const char *path)
{
	char umount_command[256];

	snprintf(umount_command, sizeof(umount_command), "sudo umount -f %s", path);
	system(umount_command);

	pclose(*stream);

	DBG_INFO("hls stream close success");
	return 0;
}

extern int8_t stream_hls_transmission_payloads(FILE **stream, int8_t *coptr, int32_t *packet_payloads)
{
	if (fwrite(coptr, *packet_payloads, 1, *stream) != 1)
	{
		DBG_WARN("failed to stream payloads");
		return -1;
	}

	fflush(*stream);

	return 0;
}
