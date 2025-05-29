#include "codec_aac.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

extern int8_t codec_aac_open(codec_aac *aucod, int16_t channels, int32_t sample_rate) {
	if (aacEncOpen(&aucod->encoder, 0, 0) > 0) {
		log_error("failed to open aac encoder");
		return -1;
	}

	if (aacEncoder_SetParam(aucod->encoder, AACENC_AOT, AAC_MODE) > 0) {
		log_error("failed to set aac encoder mode");
		return -1;
	}

	if (aacEncoder_SetParam(aucod->encoder, AACENC_CHANNELMODE, channels) > 0) {
		log_error("failed to set aac encoder channels");
		return -1;
	}

	if (aacEncoder_SetParam(aucod->encoder, AACENC_SAMPLERATE, sample_rate) > 0) {
		log_error("failed to set aac encoder sample rate");
		return -1;
	}

	if (aacEncoder_SetParam(aucod->encoder, AACENC_BITRATE, AAC_BIT_RATE) > 0) {
		log_error("failed to set aac encoder bit rate");
		return -1;
	}

	if (aacEncoder_SetParam(aucod->encoder, AACENC_TRANSMUX, AAC_TRANSMUX) > 0) {
		log_error("failed to set aac encoder transport multiplexing");
		return -1;
	}

	if (aacEncEncode(aucod->encoder, NULL, NULL, NULL, NULL) > 0) {
		log_error("failed to prepare aac encoder");
		return -1;
	}

	aucod->decoder = aacDecoder_Open(AAC_TRANSMUX, 1);
	if (!aucod->decoder) {
		log_error("failed to open aac decoder");
		return -1;
	}

	log_debug("aac codec open success");
	return 0;
}

extern int8_t codec_aac_close(codec_aac *aucod) {
	aacEncClose(&aucod->encoder);
	aacDecoder_Close(aucod->decoder);
	log_debug("aac codec close success");
	return 0;
}

extern int8_t codec_aac_encode(codec_aac *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_samples, int32_t *packet_payloads) {
	int32_t inarg_el = sizeof(int16_t);
	int32_t outarg_el = sizeof(int8_t);
	int32_t inarg_size = *packet_samples * inarg_el;
	int32_t inarg_id = IN_AUDIO_DATA;
	int32_t outarg_id = OUT_BITSTREAM_DATA;
	int32_t outarg_size = AAC_BUFFER_PAYLOADS;
	void *in_ptr = auptr;
	void *out_ptr = coptr;
	AACENC_BufDesc aac_in;
	AACENC_BufDesc aac_out;
	AACENC_InArgs aac_inargs;
	AACENC_OutArgs aac_outargs;
	aac_in.numBufs = 1;
	aac_in.bufs = &in_ptr;
	aac_in.bufferIdentifiers = &inarg_id;
	aac_in.bufSizes = &inarg_size;
	aac_in.bufElSizes = &inarg_el;
	aac_inargs.numInSamples = *packet_samples;
	aac_out.numBufs = 1;
	aac_out.bufs = &out_ptr;
	aac_out.bufferIdentifiers = &outarg_id;
	aac_out.bufSizes = &outarg_size;
	aac_out.bufElSizes = &outarg_el;
	if (aacEncEncode(aucod->encoder, &aac_in, &aac_out, &aac_inargs, &aac_outargs) > 0) {
		log_error("failed to encode aac payloads");
		return -1;
	}

	*packet_payloads = aac_outargs.numOutBytes;
	return 0;
}

extern int8_t codec_aac_decode(codec_aac *aucod, int16_t *auptr, int8_t *coptr, int32_t *packet_samples, int32_t *packet_payloads) {
	int8_t *enptr = coptr;
	int32_t *envalid = packet_payloads;
	if (aacDecoder_Fill(aucod->decoder, &enptr, packet_payloads, envalid) > 0) {
		log_error("failed to fill aac payloads");
		return -1;
	}

	if (aacDecoder_DecodeFrame(aucod->decoder, auptr, AUD_BUFFER_FRAMES * AUD_CHANNELS, 0) > 0) {
		log_error("failed to decode aac payloads");
		return -1;
	}

	*packet_samples = AUD_BUFFER_FRAMES * AUD_CHANNELS;
	return 0;
}
