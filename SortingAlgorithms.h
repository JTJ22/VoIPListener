#ifndef SORTING_ALGORITHMS_H_
#define SORTING_ALGORITHMS_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

int16_t decode_A_law(int8_t recValue);
void rtp_filtering(uint8_t* recPacket, int packLength);
#endif // SORTING_ALGORITHMS_H_
 