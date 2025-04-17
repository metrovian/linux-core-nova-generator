#pragma once
#include <alsa/asoundlib.h>

#define AUD_DEVICE_CAPTURE	"default"
#define AUD_DEVICE_PLAYBACK	"default"

#define AUD_BUFFER_FRAMES	44100

#define AUD_FORMAT		SND_PCM_FORMAT_S16_LE
#define AUD_MODE_CAPTURE	SND_PCM_STREAM_CAPTURE
#define AUD_MODE_PLAYBACK	SND_PCM_STREAM_PLAYBACK

typedef struct
{
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
} audio_device;

extern int8_t audio_device_open(audio_device *audev, int8_t aumod, int16_t channels, int32_t sample_rate);
extern int8_t audio_device_close(audio_device *audev);
extern int8_t audio_device_read_frames(audio_device *audev, int16_t *auptr, int32_t *read_frames);
extern int8_t audio_device_write_frames(audio_device *audev, int16_t *auptr, int32_t *write_frames);

