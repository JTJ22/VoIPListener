#ifndef SORTING_ALGORITHMS_H_
#define SORTING_ALGORITHMS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int16_t decode_A_law(uint8_t rec_value);
void rtp_filtering(uint8_t* rec_packet, int pack_length);
#endif // SORTING_ALGORITHMS_H_
 