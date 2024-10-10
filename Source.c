#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "SocketRec.h"

uint8_t rtp_packet[12] =
{
		0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xDE, 0xAD, 0xBE, 0xFF
};

uint8_t linear_to_alaw(int16_t pcm_val)
{
	const uint8_t ALAW_MAX = 0x7F;
	const int16_t BIAS = 0x84;
	uint8_t sign = 0;
	uint8_t exponent = 0;
	uint8_t mantissa = 0;
	uint8_t alaw_val = 0;
	int16_t mask = 0x1000;

	if (pcm_val < 0)
	{
		pcm_val = -pcm_val;
		sign = 0x80;
	}

	pcm_val += BIAS;

	if (pcm_val > 32635)
	{
		pcm_val = 32635;
	}

	for (exponent = 7; (pcm_val & mask) == 0 && exponent > 0; exponent--, mask >>= 1);

	mantissa = (pcm_val >> ((exponent == 0) ? 4 : (exponent + 3))) & 0x0F;
	alaw_val = sign | (exponent << 4) | mantissa;

	return alaw_val ^ 0xD5;
}

int main()
{
	const int sample_rate = 44100;
	const int duration_seconds = 5;
	const int num_samples = sample_rate * duration_seconds;
	const int rtp_header_size = 12;

	int16_t* array = (int16_t*)malloc((rtp_header_size + num_samples) * sizeof(int16_t));
	if (array == NULL)
	{
		fprintf(stderr, "Memory allocation failed\n");
		return 1;
	}

	memcpy(array, rtp_packet, rtp_header_size);

	for (int i = 0; i < num_samples; i++)
	{
		double t = (double)i / sample_rate;
		int16_t sample = (int16_t)(32767 * sin(2.0 * 3.14 * 261.63 * t));
		array[rtp_header_size + i] = linear_to_alaw(sample);
	}

	int packet_length = rtp_header_size + num_samples;
	rtp_filtering((uint8_t*)array, packet_length);

	start_listening();
	free(array);

	return 0;
}