#ifndef RTSPHANDLE_H_
#define RTSPHANDLE_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "PcmBuffer.h"
#include "AudioFileCreator.h"
#include "GlobalVariables.h"
#include "SocketRec.h"

#pragma comment(lib, "Ws2_32.lib")

typedef struct
{
	char session_name[64];
	char origin_ip[32];
	int port;
	char media_type[16];
	char codec[32];
} sdp_data;

typedef struct
{
	char method[16];
	char uri[256];
	int cseq;
	char session[32];
	int content_length;
	char version[32];
	char ip_address[32];
} rtsp_data;

typedef struct
{
	int port_number;
	const char* ip_address;
	const char* path;
	volatile bool* keep_running;
} socket_param;

typedef struct
{
	HANDLE thread_handle;
	socket_param* sock_pa;
} wait_param;


void process_rtsp_packet(uint8_t* buffer, int packet_length, volatile bool* is_complete, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path);
void process_announce(char* message_buffer, rtsp_data* rtsp, int packet_length, SOCKET udpSocket, struct sockaddr_in* client, int clientLen);
void process_sdp_data(const char* sdp_info, sdp_data* sdp);
void process_setup(char* message_buffer, rtsp_data* rtsp, int packet_length, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path, volatile bool* keep_run);
void send_announce_reply(sdp_data* sdp, rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen);
void send_generic_reply(rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen);
void send_setup_reply(rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path, volatile bool* keep_run);
void process_generic(char* message_buffer, rtsp_data* rtsp, int packet_length, SOCKET udpSocket, struct sockaddr_in* client, int clientLen);
int send_reply(SOCKET udpSocket, char* response, struct sockaddr_in* client, int clientLen);
int create_media_thread(const char* path, volatile bool* keep_run, char* ip_address);
DWORD WINAPI wait_for_listener_thread(LPVOID lpParam);
DWORD WINAPI start_listening_thread(LPVOID lpParam);
#endif // RTSPHANDLE_H_