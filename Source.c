#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "SocketRec.h"

uint8_t rtp_packet[] =
{
    0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xDE, 0xAD, 0xBE, 0xEF, 0xD5, 0x75, 0x25, 0xF5, 0x85, 0x95
};

int main()
{
  int packet_length = sizeof(rtp_packet) / sizeof(rtp_packet[0]);
  rtp_filtering(rtp_packet, packet_length);
	start_listening();
	return 0;
}