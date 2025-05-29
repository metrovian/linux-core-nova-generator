#include "format_wav.h"
#include "wrapper_spdlog.h"
#include "predefined.h"

static int32_t format_wav_byte_rate(int16_t channels, int32_t sample_rate) {
	return sample_rate * channels * 2;
}

static int16_t format_wav_block_align(int16_t channels) {
	return channels * 2;
}

static int32_t format_wav_data_size(int32_t data_frames) {
	return data_frames * sizeof(int16_t);
}

extern int8_t format_wav_read_file(const char *name, int16_t **auptr, int16_t *channels, int32_t *sample_rate, int32_t *read_frames) {
	FILE *fptr = fopen(name, "rb");
	if (!fptr) {
		log_error("failed to open wav file");
		return -1;
	}

	header_wav header;
	if (fread(&header, sizeof(header_wav), 1, fptr) != 1) {
		fclose(fptr);
		log_error("failed to read wav header");
		return -1;
	}

	if (strncmp((const char *)header.riff_id, "RIFF", 4) != 0) {
		fclose(fptr);
		log_critical("invalid wav header");
		return -1;
	}

	if (strncmp((const char *)header.wave_id, "WAVE", 4) != 0) {
		fclose(fptr);
		log_critical("invalid wav header");
		return -1;
	}

	if (strncmp((const char *)header.fmt_id, "fmt ", 4) != 0) {
		fclose(fptr);
		log_critical("invalid wav header");
		return -1;
	}

	if (strncmp((const char *)header.data_id, "data", 4) != 0) {
		fclose(fptr);
		log_critical("invalid wav header");
		return -1;
	}

	if (header.fmt_type != 1) {
		fclose(fptr);
		log_critical("invalid audio format");
		return -1;
	}

	if (header.bits_per_sample != 16) {
		fclose(fptr);
		log_critical("invalid bits per sample");
		return -1;
	}

	*auptr = (int16_t *)malloc(header.data_size);
	if (!(*auptr)) {
		fclose(fptr);
		log_error("failed to allocate read audio frames");
		return -1;
	}

	*read_frames = header.data_size / sizeof(int16_t);
	*sample_rate = header.sample_rate;
	*channels = header.channels;
	if (fread(*auptr, sizeof(int16_t), *read_frames, fptr) != *read_frames) {
		free(*auptr);
		fclose(fptr);
		log_error("failed to read audio frames");
		return -1;
	}

	fclose(fptr);
	log_info("%d audio frames read success", *read_frames);
	return 0;
}

extern int8_t format_wav_write_file(const char *name, int16_t **auptr, int16_t *channels, int32_t *sample_rate, int32_t *write_frames) {
	FILE *fptr = fopen(name, "wb");
	if (!fptr) {
		log_error("failed to open wav file");
		return -1;
	}

	header_wav header = {
	    .riff_id = "RIFF",
	    .riff_size = 36 + format_wav_data_size(*write_frames),
	    .wave_id = "WAVE",
	    .fmt_id = "fmt ",
	    .fmt_size = 16,
	    .fmt_type = 1,
	    .channels = *channels,
	    .sample_rate = *sample_rate,
	    .byte_rate = format_wav_byte_rate(*channels, *sample_rate),
	    .block_align = format_wav_block_align(*channels),
	    .bits_per_sample = 16,
	    .data_id = "data",
	    .data_size = format_wav_data_size(*write_frames),
	};

	if (fwrite(&header, sizeof(header_wav), 1, fptr) != 1) {
		fclose(fptr);
		log_error("failed to write wav header");
		return -1;
	}

	if (fwrite(*auptr, sizeof(int16_t), *write_frames, fptr) != *write_frames) {
		fclose(fptr);
		log_error("failed to write audio frames");
		return -1;
	}

	fclose(fptr);
	log_info("%d audio frames write success", *write_frames);
	return 0;
}
