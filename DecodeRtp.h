#ifndef DECODE_RTP_H_
#define DECODE_RTP_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <opus/opus.h>
#include "PcmBuffer.h"
#include "AudioFileCreator.h"
#include "GlobalVariables.h"
#include "decoder.h"

#define BYTES_PCM 88200

int16_t decode_A_law(uint8_t rec_value);
void rtp_filtering(uint8_t* rec_packet, int pack_length, PcmBuffer* pcm_buffer, const char* path);
int16_t decode_U_law(uint8_t rec_value);
#endif // DECODE_RTP_H_
 