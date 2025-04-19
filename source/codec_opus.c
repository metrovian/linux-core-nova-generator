#include "codec_opus.h"
#include "predefined.h"

extern int8_t codec_opus_open(codec_opus *aucod, int16_t channels, int32_t sample_rate)
{
	int32_t error_encoder = 0;
	int32_t error_decoder = 0;

	aucod->encoder = opus_encoder_create(sample_rate, channels, OPUS_MODE, &error_encoder);
	aucod->decoder = opus_decoder_create(sample_rate, channels, &error_decoder);

	if (error_encoder < 0)
	{
		DBG_WARN("failed to open encoder");
		return -1;
	}

	if (error_decoder < 0)
	{
		DBG_WARN("failed to open decoder");
		return -1;
	}

	DBG_INFO("opus codec open success");
	return 0;
}

extern int8_t codec_opus_close(codec_opus *aucod)
{
	opus_encoder_destroy(aucod->encoder);
	opus_decoder_destroy(aucod->decoder);

	DBG_INFO("opus codec close success");
	return 0;
}

extern int8_t codec_opus_encode(codec_opus *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_frames, int32_t *packet_payloads)
{
	*packet_payloads = opus_encode(aucod->encoder, auptr, *packet_frames, coptr, OPUS_BUFFER_PAYLOADS);

	if (*packet_payloads < 0)
	{
		DBG_WARN("failed to encode frames");
		return -1;
	}

	return 0;
}

extern int8_t codec_opus_decode(codec_opus *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_frames, int32_t *packet_payloads)
{
	*packet_frames = opus_decode(aucod->decoder, coptr, *packet_payloads, auptr, AUD_BUFFER_FRAMES, 0);

	if (*packet_frames < 0)
	{
		DBG_WARN("failed to decode payloads");
		return -1;
	}

	return 0;
}
