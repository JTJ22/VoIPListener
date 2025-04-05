#include "RTSPHandle.h"

/// <summary>
/// Processes an RTSP packet to handle commands
/// </summary>
/// <param name="buffer">Packet recieved to be decoded</param>
/// <param name="packet_length">The length of the packet</param>
/// <param name="is_complete">Checks if thread should remain open</param>
/// <param name="udpSocket">Socket currently handling RTSP packets</param>
/// <param name="client">Client data to handle communications</param>
/// <param name="clientLen"></param>
/// <param name="path">Path to save audio data</param>
void process_rtsp_packet(uint8_t * buffer, int packet_length, volatile bool* is_complete, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path)
{
	char message_buffer[1024];

	if(packet_length >= sizeof(message_buffer))
	{
		printf("Packet too large\n");
		return;
	}

	memcpy(message_buffer, buffer, packet_length);
	message_buffer[packet_length] = '\0';

	rtsp_data rtsp;
	memset(&rtsp, 0, sizeof(rtsp_data));

	sscanf_s(message_buffer, "%15s %127s RTSP/1.0", rtsp.method, (unsigned)_countof(rtsp.method), rtsp.uri, (unsigned)_countof(rtsp.uri));

	if(strcmp(rtsp.method, "ANNOUNCE") == 0)
	{
		process_announce(&message_buffer, &rtsp, packet_length, udpSocket, client, clientLen);
	}
	else if(strcmp(rtsp.method, "SETUP") == 0)
	{
		process_setup(&message_buffer, &rtsp, packet_length, udpSocket, client, clientLen, path, is_complete);
	}
}

/// <summary>
/// Processes an announce packet
/// </summary>
/// <param name="message_buffer"></param>
/// <param name="rtsp"></param>
/// <param name="packet_length"></param>
/// <param name="udpSocket"></param>
/// <param name="client"></param>
/// <param name="clientLen"></param>
void process_announce(char* message_buffer, rtsp_data* rtsp, int packet_length, SOCKET udpSocket, struct sockaddr_in* client, int clientLen)
{
	char* cseq_str = strstr(message_buffer, "CSeq:");
	if(cseq_str)
	{
		sscanf_s(cseq_str, "CSeq: %d", &rtsp->cseq);
	}

	char* session_str = strstr(message_buffer, "Session:");
	if(session_str)
	{
		sscanf_s(session_str, "Session: %31s", rtsp->session, (unsigned)_countof(rtsp->session));
	}

	char* content_str = strstr(message_buffer, "Content-Length:");
	if(content_str)
	{
		sscanf_s(content_str, "Content-Length: %d", &rtsp->content_length);
	}

	char* sdp_start = strstr(message_buffer, "\r\n\r\n");
	if(sdp_start)
	{
		sdp_start += 4;
		if(content_str != NULL && sdp_start < message_buffer + packet_length)
		{
			sdp_data sdp;
			process_sdp_data(sdp_start, &sdp);
			send_announce_reply(&sdp, rtsp, udpSocket, client, clientLen);
		}
	}
}

void process_setup(char* message_buffer, rtsp_data* rtsp, int packet_length, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path, volatile bool* keep_run)
{
	memset(rtsp, 0, sizeof(rtsp_data));

	char* cseq_str = strstr(message_buffer, "CSeq:");
	if(cseq_str)
	{
		sscanf_s(cseq_str, "CSeq: %d", &rtsp->cseq);
	}

	char* session_str = strstr(message_buffer, "Session:");
	if(session_str)
	{
		sscanf_s(session_str, "Session: %31s", rtsp->session, (unsigned)_countof(rtsp->session));
	}

	send_setup_reply(rtsp, udpSocket, client, clientLen, path, keep_run);
}

void process_sdp_data(const char* sdp_info, sdp_data* sdp)
{
	memset(sdp, 0, sizeof(sdp));

	const char* lines = sdp_info;
	while(lines && *lines)
	{
		const char* next_line = strchr(lines, '\n');
		if(!next_line) break;

		char buffer[256] = { 0 };
		strncpy_s(buffer, sizeof(buffer), lines, next_line - lines);

		if(strncmp(buffer, "s=", 2) == 0)
		{
			strncpy_s(sdp->session_name, sizeof(sdp->session_name), buffer + 2, _TRUNCATE);
		}

		if(strncmp(buffer, "o=", 2) == 0)
		{
			sscanf_s(buffer, "o=%*s %*d %*d IN IP4 %31s", sdp->origin_ip, (unsigned)_countof(sdp->origin_ip));
		}

		if(strncmp(buffer, "m=", 2) == 0)
		{
			sscanf_s(buffer, "m=%15s %d %*s %*d", sdp->media_type, (unsigned)_countof(sdp->media_type), &sdp->port);
		}

		if(strncmp(buffer, "a=rtpmap:", 9) == 0)
		{
			sscanf_s(buffer, "a=rtpmap:%*d %31s", sdp->codec, (unsigned)_countof(sdp->codec));
		}

		lines = next_line + 1;
	}
}

void send_announce_reply(sdp_data* sdp, rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen)
{
	char response[512];
	char my_ip[32] = "192.168.1.1";
	unsigned long content_length = (unsigned long)strlen(sdp->session_name) + 89;

	snprintf(response, sizeof(response),
		"RTSP/1.0 200 OK\r\n"
		"WG67-Version: recorder.02\r\n"
		"CSeq: %d\r\n"
		"Session: %s\r\n"
		"Content-Type: application/sdp\r\n"
		"Content-Length: %lu\r\n"
		"\r\n"
		"v=0\r\n"
		"o=Recorder1 0 1 IN IP4 %s\r\n"
		"s=%s\r\n"
		"c=IN IP4 %s\r\n"
		"t=0 0\r\n"
		"m=%s %d RTP/AVP 8\r\n"
		"a=rtpmap:8 %s\r\n",
		rtsp->cseq,
		rtsp->session[0] ? rtsp->session : "NEW_SESSION",
		content_length,
		sdp->origin_ip,
		sdp->session_name,
		my_ip,
		sdp->media_type,
		sdp->port,
		sdp->codec);

	send_reply(udpSocket, response, client, clientLen);
}

void send_setup_reply(rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path, volatile bool* keep_run)
{
	char response[512];
	char my_ip[32] = "192.168.1.1"; 

	snprintf(response, sizeof(response),
		"RTSP/1.0 200 OK\r\n"
		"WG67-Version: recorder.02\r\n"
		"CSeq: %d\r\n"
		"Transport: RTP/AVP/UDP;src_addr=\"%s:6000\";setup=passive\r\n"
		"Session: %s;timeout=60\r\n"
		"\r\n",
		rtsp->cseq,
		my_ip,
		rtsp->session);

	send_reply(udpSocket, response, client, clientLen);
	create_media_thread(path, keep_run);
}

int send_reply(SOCKET udpSocket, char* response, struct sockaddr_in* client, int clientLen)
{
	int sentBytes = sendto(udpSocket, response, strlen(response), 0, (struct sockaddr*)client, clientLen);

	if(sentBytes == SOCKET_ERROR)
	{
		printf("Failed to send response. Error code: %d\n", WSAGetLastError());
		return -1;
	}
	else
	{
		return 0;
	}
}

int create_media_thread(const char* path, volatile bool* keep_run)
{
	socket_param* sock_pa = (socket_param*)malloc(sizeof(socket_param));

	if(sock_pa == NULL) 
	{
		printf("Failed to allocate memory.\n");
		return 1;
	}

	sock_pa->ip_address = "127.0.0.1";
	sock_pa->path = path;
	sock_pa->port_number = 6000;
	sock_pa->keep_running = keep_run;

	HANDLE listenerThread = CreateThread(NULL, 0, start_listening_thread, sock_pa, 0, NULL);

	if(listenerThread == NULL)
	{
		printf("Failed to create listener thread.\n");
		free(sock_pa);
		return 1;
	}

	CloseHandle(listenerThread);

	return 0;
}

DWORD WINAPI start_listening_thread(LPVOID lpParam)
{
	socket_param* args = (socket_param*)lpParam;

	int result = start_listening_media(args->ip_address, &args->port_number, &args->keep_running, args->path);
	free(args);
	return result;
}