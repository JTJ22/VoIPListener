#include "SortingAlgorithms.h"

void rtp_filtering(uint8_t *recPacket, int packLength)
{
	if(packLength < 12)
	{
		printf("Not big enough for VoIP packet.");
		return;
	}

	uint8_t rtp_version = (recPacket[0] >> 6) & 0x03;
	uint8_t rtp_type = recPacket[1] & 0x7F;

	if(rtp_version != 2)
	{
		printf("Invalid RTP version.%d", rtp_version);
		return;
	}

	uint8_t* message = recPacket + 12;
	int message_length = packLength - 12;

	switch (rtp_type)
	{
	case 8:
		for(int i = 0; i < message_length; i++)
		{
			int16_t decoded_sample = decode_A_law(message[i]);
		}
		break;
	case 0:
		break;
	default:
		printf("Unrecognised RTP packet");
	}

	
}

int16_t decode_A_law(int8_t recValue)
{
	uint8_t posOrNeg = 0x00;
	uint8_t exponent = 0;
	uint16_t decodedValue = 0x00;

	recValue ^= 0x55;

	if(recValue & 0x80)
	{
		recValue &= ~(1 << 7);
		posOrNeg = -1;
	}

	exponent = ((recValue & 0xF0) >> 4) + 4;

	if(exponent != 4)
	{
		decodedValue = ((1 << exponent) | ((recValue & 0x0F) << (exponent - 4)) | (1 << (exponent - 5)));
	}
	else
	{
		decodedValue = (recValue << 1) | 1;
	}

	return (posOrNeg == 0) ? (decodedValue) : (-decodedValue);
}