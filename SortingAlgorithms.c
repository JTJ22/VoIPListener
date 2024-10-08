#include "SortingAlgorithms.h"
#include "AudioFileCreator.h"

/// <summary>
/// Filters a packet based upon the rtp version
/// </summary>
/// <param name="recPacket">The packet being checked</param>
/// <param name="packLength">The packets length</param>
void rtp_filtering(uint8_t* rec_packet, int pack_length)
{
	int16_t pcm_buffer[20480];
	int recvLen;
	int pcm_buffer_len = 0;
	int packet_count = 0;

	if (pack_length < 12)
	{
		printf("Not big enough for VoIP packet.");
		return;
	}

	uint8_t rtp_version = (rec_packet[0] >> 6) & 0x03;
	uint8_t rtp_type = rec_packet[1] & 0x7F;

	if (rtp_version != 2)
	{
		printf("Invalid RTP version.%d", rtp_version);
		return;
	}

	uint8_t* message = rec_packet + 12;
	int message_length = pack_length - 12;

	switch (rtp_type)
	{
	case 8:
		for (int i = 0; i < message_length; i++)
		{
			int16_t decoded_sample = decode_A_law(message[i]);
			pcm_buffer[pcm_buffer_len] = decoded_sample;
			pcm_buffer_len++;
		}
		packet_count++;
		break;
	case 0:
		break;
	default:
		printf("Unrecognised RTP packet");
	}

	save_wav_file(&pcm_buffer_len, pcm_buffer, &packet_count);
}

/// <summary>
/// Decodes G.711 A-law using the decoding algorithm
/// </summary>
/// <param name="recValue">The byte being checked</param>
/// <returns>A 16 bit value containing audio information</returns>
int16_t decode_A_law(uint8_t rec_value)
{
	uint8_t pos_or_neg = 0x00;
	uint8_t exponent = 0;
	uint16_t decoded_value = 0x00;
	rec_value;
	rec_value ^= 0x55;

	if(rec_value & 0x80)
	{
		rec_value &= ~(1 << 7);
		pos_or_neg = -1;
	}

	decoded_value = ((rec_value & 0x0F) << 4);
	exponent = ((rec_value & 0x70) >> 4);

	switch(exponent)
	{
	case 0:
		decoded_value += 8;
		break;
	case 1:
		decoded_value += 0x108;
		break;
	default:
		decoded_value += 0x108;
		decoded_value <<= exponent - 1;
		break;
	}

	return (pos_or_neg == 0) ? (decoded_value) : (-decoded_value);
}