#pragma once
#include <stdio.h>
#include <stdlib.h>

extern int8_t stream_janus_open(FILE **stream, const char *path);
extern int8_t stream_janus_close(FILE **stream);
extern int8_t stream_janus_transmission_payloads(FILE **stream, int8_t *coptr, int32_t *packet_payloads);

