#ifndef SORTING_ALGORITHMS_H_
#define SORTING_ALGORITHMS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define BYTES_PCM 88200
#define SAMPLE_RATE 44100
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define DURATION 5
#define BUFFER_SIZE (SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8) * DURATION)


int16_t decode_A_law(uint8_t rec_value);
void rtp_filtering(uint8_t* rec_packet, int pack_length);
int16_t decode_U_law(uint8_t rec_value);
#endif // SORTING_ALGORITHMS_H_
 