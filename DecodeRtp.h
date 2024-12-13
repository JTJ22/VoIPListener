#ifndef DECODE_RTP_H_
#define DECODE_RTP_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "PcmBuffer.h"
#include "AudioFileCreator.h"
#include "GlobalVariables.h"

#define BYTES_PCM 88200

int16_t decode_A_law(uint8_t rec_value);
void rtp_filtering(uint8_t* rec_packet, int pack_length, PcmBuffer* pcm_buffer);
int16_t decode_U_law(uint8_t rec_value);
#endif // DECODE_RTP_H_
 