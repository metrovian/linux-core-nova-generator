#include "audio_device.h"
#include "predefined.h"

extern int8_t audio_device_open(audio_device *audev, int8_t aumod, int16_t channels, int32_t sample_rate)
{	
	if (aumod == AUD_MODE_CAPTURE)
	{
		if (snd_pcm_open(&audev->handle, AUD_DEVICE_CAPTURE, aumod, 0) < 0)
		{
			DBG_WARN("failed to open audio capture device");
			return -1;
		}
	}

	else if (aumod == AUD_MODE_PLAYBACK)
	{
		if (snd_pcm_open(&audev->handle, AUD_DEVICE_PLAYBACK, aumod, 0) < 0)
		{
			DBG_WARN("failed to open audio playback device");
			return -1;
		}
	}

	else
	{	
		DBG_WARN("invalid audio device");
		return -1;
	}

	if (snd_pcm_hw_params_malloc(&audev->params) < 0)
	{
		DBG_WARN("failed to allocate hardware parameters");
		return -1;
	}

	if (snd_pcm_hw_params_any(audev->handle, audev->params) < 0)
	{
		DBG_WARN("failed to initialize hardware parameters");
		return -1;
	}

	if (snd_pcm_hw_params_set_access(audev->handle, audev->params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{
		DBG_WARN("failed to set access type");
		return -1;
	}
	
	if (snd_pcm_hw_params_set_format(audev->handle, audev->params, AUD_FORMAT) < 0)
	{
		DBG_WARN("failed to set format");
		return -1;
	}

	if (snd_pcm_hw_params_set_channels(audev->handle, audev->params, channels) < 0)
	{
		DBG_WARN("failed to set channels");
		return -1;
	}

	if (snd_pcm_hw_params_set_rate(audev->handle, audev->params, sample_rate, 0) < 0)
	{
		DBG_WARN("failed to set sample rate");
		return -1;
	}

	if (snd_pcm_hw_params(audev->handle, audev->params) < 0)
	{
		DBG_WARN("failed to set hardware parameters");
		return -1;
	}

	if (snd_pcm_prepare(audev->handle) < 0)
	{
		DBG_WARN("failed to prepare audio device");
		return -1;
	}

	if (aumod == AUD_MODE_CAPTURE)
	{
		DBG_INFO("audio capture device open success");
		return 0;
	}

	else if (aumod == AUD_MODE_PLAYBACK)
	{
		DBG_INFO("audio playback device open success");
		return 0;
	}

	return 0;
}

extern int8_t audio_device_close(audio_device *audev)
{
	snd_pcm_hw_params_free(audev->params);
	snd_pcm_drain(audev->handle);
	snd_pcm_close(audev->handle);

	DBG_INFO("audio device close success");
	return 0;
}

extern int8_t audio_device_read_frames(audio_device *audev, int16_t *auptr, int32_t *read_samples)
{
	snd_pcm_sframes_t frames = snd_pcm_readi(audev->handle, auptr, *read_samples);
	
	if (frames < 0)
	{
		frames = snd_pcm_recover(audev->handle, frames, 1);

		if (frames < 0)
		{
			snd_pcm_prepare(audev->handle);

			DBG_WARN("audio capture failed");
			return -1;
		}
	}

	int32_t channels = 1;	
	int32_t error = snd_pcm_hw_params_get_channels(audev->params, &channels);

	if (error < 0)
	{
		DBG_WARN("failed to get channels");
		return -1;
	}

	*read_samples = frames * channels;

	return 0;
}

extern int8_t audio_device_write_frames(audio_device *audev, int16_t *auptr, int32_t *write_samples)
{
	int32_t channels = 1;	
	int32_t error = snd_pcm_hw_params_get_channels(audev->params, &channels);

	if (error < 0)
	{
		DBG_WARN("failed to get channels");
		return -1;
	}

	snd_pcm_sframes_t frames = snd_pcm_writei(audev->handle, auptr, *write_samples / channels);
	
	if (frames < 0)
	{
		frames = snd_pcm_recover(audev->handle, frames, 1);

		if (frames < 0)
		{
			snd_pcm_prepare(audev->handle);

			DBG_WARN("audio playback failed");
			return -1;
		}
	}

	return 0;
}
