#include "RTSPHandle.h"

/// <summary>
/// Processes an RTSP packet to handle commands
/// </summary>
/// <param name="buffer">Packet recieved to be decoded</param>
/// <param name="packet_length">The length of the packet</param>
/// <param name="is_complete">Checks if thread should remain open</param>
/// <param name="udpSocket">Socket currently handling RTSP packets</param>
/// <param name="client">Client data to handle communications</param>
/// <param name="clientLen">Defines how long data about the client is</param>
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
	else if(strcmp(rtsp.method, "RECORD") == 0)
	{
		process_generic(&message_buffer, &rtsp, packet_length, udpSocket, client, clientLen);
	}
	else if(strcmp(rtsp.method, "PAUSE") == 0)
	{
		process_generic(&message_buffer, &rtsp, packet_length, udpSocket, client, clientLen);
	}
	else if(strcmp(rtsp.method, "TEARDOWN") == 0)
	{
		process_generic(&message_buffer, &rtsp, packet_length, udpSocket, client, clientLen);
	}
	else if(strcmp(rtsp.method, "GET_PARAMETER") == 0)
	{
		process_generic(&message_buffer, &rtsp, packet_length, udpSocket, client, clientLen);
	}
	else if(strcmp(rtsp.method, "SET_PARAMETER") == 0)
	{
		process_generic(&message_buffer, &rtsp, packet_length, udpSocket, client, clientLen);
	}
	else
	{
		printf("Unknown RTSP method: %s\n", rtsp.method);
	}

}

/// <summary>
/// Processes an announce packet
/// </summary>
/// <param name="message_buffer">Contains packet data</param>
/// <param name="rtsp">Struct to contain data</param>
/// <param name="packet_length">Lenght of packet</param>
/// <param name="udpSocket">Socket to send data through</param>
/// <param name="client">Client information</param>
/// <param name="clientLen">Defines how long data about the client is</param>
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

	char* ver_str = strstr(message_buffer, "WG67-Version:");
	if(ver_str)
	{
		sscanf_s(ver_str, "WG67-Version: %31s", rtsp->version, (unsigned)_countof(rtsp->version));
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

/// <summary>
/// Processes a SETUP packet received via RTSP
/// </summary>
/// <param name="message_buffer">The packet that has been received</param>
/// <param name="rtsp">Struct containing rtsp information</param>
/// <param name="packet_length">Size of the packet</param>
/// <param name="udpSocket">Socket to return messages</param>
/// <param name="client">Client information</param>
/// <param name="clientLen">The length of the clients information</param>
/// <param name="path">Path to save audio data to</param>
/// <param name="keep_run">Boolean to keep socket running</param>
void process_setup(char* message_buffer, rtsp_data* rtsp, int packet_length, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path, volatile bool* keep_run)
{
	memset(rtsp, 0, sizeof(rtsp_data));

	char* cseq_str = strstr(message_buffer, "CSeq:");
	if(cseq_str)
	{
		sscanf_s(cseq_str, "CSeq: %d", &rtsp->cseq);
	}

	char* ver_str = strstr(message_buffer, "WG67-Version:");
	if(ver_str)
	{
		sscanf_s(ver_str, "WG67-Version: %31s", rtsp->version, (unsigned)_countof(rtsp->version));
	}

	char* session_str = strstr(message_buffer, "Session:");
	if(session_str)
	{
		sscanf_s(session_str, "Session: %31s", rtsp->session, (unsigned)_countof(rtsp->session));
	}

	char* rtsp_url = strstr(message_buffer, "rtsp://");
	if(rtsp_url)
	{
		rtsp_url += 7;
		sscanf_s(rtsp_url, "%15[^:]", rtsp->ip_address, (unsigned)_countof(rtsp->ip_address));
	}

	send_setup_reply(rtsp, udpSocket, client, clientLen, path, keep_run);
}

/// <summary>
/// Processes the SDP data from the announce packet
/// </summary>
/// <param name="sdp_info">String containing the data</param>
/// <param name="sdp">Struct to store decoded data</param>
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

/// <summary>
/// Builds a response to the ANNOUNCE command
/// </summary>
/// <param name="sdp">SDP data to respond with</param>
/// <param name="rtsp">Struct containing RTSP data</param>
/// <param name="udpSocket">Socket to respond on</param>
/// <param name="client">Client data to respond on</param>
/// <param name="clientLen">Length of client data</param>
void send_announce_reply(sdp_data* sdp, rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen)
{
	char response[512];
	char my_ip[32] = "192.168.1.1";
	unsigned long content_length = (unsigned long)strlen(sdp->session_name) + 89;

	snprintf(response, sizeof(response),
		"RTSP/1.0 200 OK\r\n"
		"WG67-Version: %s\r\n"
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
		rtsp->version,
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

/// <summary>
/// Builds a response to a SETUP command
/// </summary>
/// <param name="rtsp">Struct containing RTSP data</param>
/// <param name="udpSocket">Socket to respond on</param>
/// <param name="client">Client data to respond to</param>
/// <param name="clientLen">Length of client data</param>
/// <param name="path">Path to store audio data</param>
/// <param name="keep_run">Boolean to control socket use</param>
void send_setup_reply(rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen, const char* path, volatile bool* keep_run)
{
	char response[512];

	snprintf(response, sizeof(response),
		"RTSP/1.0 200 OK\r\n"
		"WG67-Version: %s\r\n"
		"CSeq: %d\r\n"
		"Transport: RTP/AVP/UDP;src_addr=\"%s:6000\";setup=passive\r\n"
		"Session: %s;timeout=60\r\n"
		"\r\n",
		rtsp->version,
		rtsp->cseq,
		rtsp->ip_address,
		rtsp->session);

	send_reply(udpSocket, response, client, clientLen);
	create_media_thread(path, keep_run, rtsp->ip_address);
}

/// <summary>
/// Processes generic RTSP commands that require a 200 ok response
/// </summary>
/// <param name="message_buffer">Message received by the socket</param>
/// <param name="rtsp">Struct containing RTSP data</param>
/// <param name="packet_length">Size of the packet</param>
/// <param name="udpSocket">Socket to respond on</param>
/// <param name="client">Client data</param>
/// <param name="clientLen">Client data length</param>
void process_generic(char* message_buffer, rtsp_data* rtsp, int packet_length, SOCKET udpSocket, struct sockaddr_in* client, int clientLen)
{
	char* cseq_str = strstr(message_buffer, "CSeq:");
	if(cseq_str)
	{
		sscanf_s(cseq_str, "CSeq: %d", &rtsp->cseq);
	}

	char* ver_str = strstr(message_buffer, "WG67-Version:");
	if(ver_str)
	{
		sscanf_s(ver_str, "WG67-Version: %31s", rtsp->version, (unsigned)_countof(rtsp->version));
	}

	char* session_str = strstr(message_buffer, "Session:");
	if(session_str)
	{
		sscanf_s(session_str, "Session: %31s", rtsp->session, (unsigned)_countof(rtsp->session));
	}

	send_generic_reply(rtsp, udpSocket, client, clientLen);
}

/// <summary>
/// Constructs a generic RTSP response to commands such as PAUSE, TEARDOWN, and GET_PARAMETER
/// </summary>
/// <param name="rtsp">Struct containing RTSP data</param>
/// <param name="udpSocket">Socket to respond on</param>
/// <param name="client">Client information to respond to</param>
/// <param name="clientLen">Size of client data</param>
void send_generic_reply(rtsp_data* rtsp, SOCKET udpSocket, struct sockaddr_in* client, int clientLen)
{
	char response[512];

	snprintf(response, sizeof(response),
		"RTSP/1.0 200 OK\r\n"
		"WG67-Version: %s\r\n"
		"CSeq: %d\r\n"
		"Session: %s\r\n"
		"\r\n",
		rtsp->version,
		rtsp->cseq,
		rtsp->session);

	send_reply(udpSocket, response, client, clientLen);
}

/// <summary>
/// Sends a constructed response back to the client
/// </summary>
/// <param name="udpSocket">Socket to respond on</param>
/// <param name="response">Data to send back to the client</param>
/// <param name="client">Client data needs to be sent to</param>
/// <param name="clientLen">Size of client data</param>
/// <returns>0 if succeeds</returns>
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

/// <summary>
/// Creates a thread to listen for media
/// </summary>
/// <param name="path">Path to save audio data to</param>
/// <param name="keep_run">Bool to control running of data</param>
/// <param name="ip_address">Ip address to listen on</param>
/// <returns>0 if successful</returns>
int create_media_thread(const char* path, volatile bool* keep_run, char* ip_address)
{
	socket_param* sock_pa = (socket_param*)malloc(sizeof(socket_param));

	if(sock_pa == NULL) 
	{
		printf("Failed to allocate memory.\n");
		return 1;
	}

	sock_pa->ip_address = ip_address;
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

	wait_param* wp = (wait_param*)malloc(sizeof(wait_param));
	if(wp == NULL)
	{
		printf("Failed to allocate waiter param.\n");
		CloseHandle(listenerThread);
		free(sock_pa);
		return 1;
	}

	wp->thread_handle = listenerThread;
	wp->sock_pa = sock_pa;

	HANDLE waiterThread = CreateThread(NULL, 0, wait_for_listener_thread, wp, 0, NULL);
	if(waiterThread == NULL)
	{
		printf("Failed to create waiter thread.\n");
		CloseHandle(listenerThread);
		free(sock_pa);
		free(wp);
		return 1;
	}

	CloseHandle(waiterThread);
	return 0;
}

/// <summary>
/// Waits for the media thread, prevents RTSP thread from blocking
/// </summary>
/// <param name="lpParam">Parameters</param>
/// <returns>0 if successful</returns>
DWORD WINAPI wait_for_listener_thread(LPVOID lpParam)
{
	wait_param* wp = (wait_param*)lpParam;
	WaitForSingleObject(wp->thread_handle, INFINITE);

	CloseHandle(wp->thread_handle);
	free(wp->sock_pa);
	free(wp);

	return 0;
}

/// <summary>
/// Starts a thread to listen for media data.
/// </summary>
/// <param name="lpParam">Parameters to be passed to listener</param>
/// <returns>Result of listener</returns>
DWORD WINAPI start_listening_thread(LPVOID lpParam)
{
	socket_param* args = (socket_param*)lpParam;

	int result = start_listening_media(args->ip_address, &args->port_number, args->keep_running, args->path);
	return result;
}