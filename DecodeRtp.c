#include "DecodeRtp.h"

/// <summary>
/// Filters a packet based upon the rtp version
/// </summary>
/// <param name="recPacket">The packet being checked</param>
/// <param name="packLength">The packets length</param>
void rtp_filtering(uint8_t* rec_packet, int pack_length, PcmBuffer* pcm_buffer, const char* path)
{

	if(pack_length < 12)
	{
		printf("Not big enough for VoIP packet.");
		return;
	}

	uint16_t sequence_number = (rec_packet[2] << 8) | rec_packet[3];
	uint8_t rtp_version = (rec_packet[0] >> 6) & 0x03;
	uint8_t rtp_type = rec_packet[1] & 0x7F;

	if(rtp_version != 2)
	{
		printf("Invalid RTP version. %d", rtp_version);
		return;
	}

	uint8_t* message = rec_packet + 12;
	int message_length = pack_length - 12;

	if(rtp_type == 8 || rtp_type == 0)
	{
		int16_t* pcm_data = (int16_t*)malloc(message_length * sizeof(int16_t));
		if(pcm_data == NULL)
		{
			printf("Malloc failed.");
			return;
		}

		for(int i = 0; i < message_length; i++)
		{
			pcm_data[i] = (rtp_type == 8) ? decode_A_law(message[i]) : decode_U_law(message[i]);
		}

		if(pcm_buffer->length + message_length <= pcm_buffer->max_length)
		{
			add_to_PCM_buffer(pcm_buffer, pcm_data, message_length);
		}
		else
		{
			process_PCM_buffer(pcm_buffer, path);
			add_to_PCM_buffer(pcm_buffer, pcm_data, message_length);
		}

		free(pcm_data);
	}
	else if(rtp_type == 18)
	{
		int16_t pcm_data[80];
		bcg729DecoderChannelContextStruct* g729_decoder = initBcg729DecoderChannel();
		int num_frames = message_length / 10;
		if(message_length % 10 != 0)
		{
			printf("Invalid G.729 packet size");
			return;
		}

		for(int i = 0; i < num_frames; i++)
		{
			bcg729Decoder(g729_decoder, message + i * 10, 10, 0, 0, 0, pcm_data);

			if(pcm_buffer->length + 80 <= pcm_buffer->max_length)
			{
				add_to_PCM_buffer(pcm_buffer, pcm_data, 80);
			}
			else
			{
				process_PCM_buffer(pcm_buffer, path);
				add_to_PCM_buffer(pcm_buffer, pcm_data, 80);
			}
		}

		closeBcg729DecoderChannel(g729_decoder);
	}
	else
	{
		printf("Unrecognised RTP");
	}
}

/// <summary>
/// Decodes G.711 A-law using the decoding algorithm
/// </summary>
/// <param name="recValue">The byte being checked</param>
/// <returns>A 16 bit value containing audio information</returns>
int16_t decode_A_law(uint8_t rec_value)
{
	rec_value ^= 0x55;
	bool is_pos = (rec_value & 0x80) ? false : true;
	rec_value &= 0x7F;

	uint16_t decoded_value = ((rec_value & 0x0F) << 4);
	uint8_t exponent = (rec_value & 0x70) >> 4;

	if(exponent > 1)
	{
		decoded_value += 0x108;
		decoded_value <<= (exponent - 1);
	}
	else
	{
		decoded_value += (exponent == 0) ? 8 : 0x108;
	}

	return (is_pos) ? decoded_value : -decoded_value;
}

/// <summary>
/// Decodes an encoded U-law byte into a 16 bit PCM value
/// </summary>
/// <param name="rec_value">The encoded byte being converted back</param>
/// <returns>A PCM value decoded</returns>
int16_t decode_U_law(uint8_t rec_value)
{
	rec_value = ~rec_value;

	bool is_pos = (rec_value & 0x80) ? false : true;
	rec_value &= 0x7F;

	uint8_t exponent = (rec_value >> 4) & 0x07;
	uint8_t mantissa = rec_value & 0x0F;

	uint16_t decoded_value = (mantissa << 3) + 0x84;
	decoded_value <<= exponent;

	return (is_pos) ? decoded_value : -decoded_value;
}