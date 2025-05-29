#include "format_aac.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

static const int32_t ADTS_TABLE_SAMPLE_RATE[16] = {
    96000,
    88200,
    64000,
    48000,
    44100,
    32000,
    24000,
    22050,
    16000,
    12000,
    11025,
    8000,
    7350,
    0,
    0,
    0,
};

extern int8_t format_aac_read_file(const char *name, int8_t **coptr, int16_t *channels, int32_t *sample_rate, int32_t *read_payloads) {
	FILE *fptr = fopen(name, "rb");
	if (!fptr) {
		log_error("failed to open aac file");
		return -1;
	}

	int8_t header[7];
	if (fread(&header, sizeof(header), 1, fptr) != 1) {
		fclose(fptr);
		log_error("failed to read adts header");
		return -1;
	}

	if ((header[0] & 0xFF) != 0xFF) {
		fclose(fptr);
		log_critical("invalid adts header");
		return -1;
	}

	if ((header[1] & 0xF0) != 0xF0) {
		fclose(fptr);
		log_critical("invalid adts header");
		return -1;
	}

	int32_t rate_index = ((header[2] & 0x3C) >> 2);
	int32_t channels_msb = ((header[2] & 0x01) << 2);
	int32_t channels_lsb = ((header[3] & 0x3C) >> 2);
	int32_t payloads_msb = ((header[3] & 0x03) << 1);
	int32_t payloads_csb = ((header[4] & 0xFF) << 3);
	int32_t payloads_lsb = ((header[5] & 0xE0) >> 5);
	int16_t header_channels = channels_msb | channels_lsb;
	int32_t header_payloads = payloads_msb | payloads_csb | payloads_lsb;
	*coptr = (int8_t *)malloc(header_payloads);
	*channels = header_channels;
	*read_payloads = header_payloads;
	*sample_rate = ADTS_TABLE_SAMPLE_RATE[rate_index];
	if (fread(coptr, header_payloads, 1, fptr) != 1) {
		free(*coptr);
		fclose(fptr);
		log_error("failed to read aac payloads");
		return -1;
	}

	fclose(fptr);
	log_info("%d aac payloads read success", *read_payloads);
	return 0;
}

extern int8_t format_aac_write_file(const char *name, int8_t **coptr, int16_t *channels, int32_t *sample_rate, int32_t *write_payloads) {
	FILE *fptr = fopen(name, "wb");
	if (!fptr) {
		log_error("failed to open aac file");
		return -1;
	}

	if (fwrite(*coptr, sizeof(int8_t), *write_payloads, fptr) != *write_payloads) {
		fclose(fptr);
		log_error("failed to write aac payloads");
		return -1;
	}

	fclose(fptr);
	log_info("%d aac payloads write success", *write_payloads);
	return 0;
}
