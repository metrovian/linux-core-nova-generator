#pragma once
#include <alsa/asoundlib.h>

typedef struct
{
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
} audio_device;

extern int8_t audio_device_open(audio_device *audev, int8_t aumod, int16_t channels, int32_t sample_rate);
extern int8_t audio_device_close(audio_device *audev);
extern int8_t audio_device_read_frames(audio_device *audev, int16_t *auptr, int32_t *read_frames);
extern int8_t audio_device_write_frames(audio_device *audev, int16_t *auptr, int32_t *write_frames);

