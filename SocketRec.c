#include "SocketRec.h"

static CallbackFunction s_callback = NULL;

/// <summary>
/// 
/// </summary>
/// <param name="callback"></param>
__declspec(dllexport) void event_register(CallbackFunction callback)
{
	s_callback = callback;
}

/// <summary>
/// 
/// </summary>
/// <param name="message"></param>
void trigger_data_rec(const char* message)
{
	if(s_callback != NULL)
	{
		s_callback(message);
	}
}

/// <summary>
/// Creates a socket that will be used to receive VoIP data.
/// </summary>
/// <returns>A UDP socket, integer value</returns>
SOCKET create_socket()
{
	SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if(udpSocket == INVALID_SOCKET)
	{
		printf("Cannot Create Socket: %d\n", WSAGetLastError());
		WSACleanup();
		return INVALID_SOCKET;
	}

	u_long mode = 1;
	if(ioctlsocket(udpSocket, FIONBIO, &mode) != NO_ERROR)
	{
		printf("Cannot set socket to non-blocking: %d\n", WSAGetLastError());
		closesocket(udpSocket);
		WSACleanup();
		return INVALID_SOCKET;
	}

	return udpSocket;
}

/// <summary>
/// WSA start up is required to allow sockets to function
/// specified the version of WinSock to use
/// </summary>
/// <returns>1 if fails, 0 if succesful</returns>
int wsa_startup()
{
	WSADATA wsaData;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Cannot start up WSA: %d\n", WSAGetLastError());
		return 1;
	}

	return 0;
}

/// <summary>
/// Assigns address type, port and IP, used later to bind to the socket
/// </summary>
/// <param name="addr">Data structure containing connection details</param>
/// <param name="port">Port number to listen on</param>
void socket_address_add(struct sockaddr_in* addr, int port, const char* ip_address)
{
	memset(addr, 0, sizeof(*addr));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);

	if(ip_address == NULL || strcmp(ip_address, "0.0.0.0") == 0)
	{
		addr->sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		int result = inet_pton(AF_INET, ip_address, &addr->sin_addr);
		if(result != 1)
		{
			if(result == 0)
			{
				printf("Invalid IP address format: %s\n", ip_address);
			}
			else
			{
				printf("inet_pton failed: %d\n", WSAGetLastError());
			}
		}
	}
}

/// <summary>
/// When ctrl-c is pressed, the socket is closed
/// </summary>
/// <param name="conSig">Input for ctrl-c</param>
void stop_socket(int conSig)
{
	printf("Closing socket.\n");
	// keep_running = false;
}

/// <summary>
/// Creates a looping buffer to recieve packets via UDP
/// </summary>
/// <returns>0 or 1 based on performance</returns>
int start_listening(const char* ip_address, int* port_no, volatile bool* keep_running, const char* path)
{
	struct sockaddr_in server, client;
	uint8_t buffer[173];

	if(wsa_startup() == 1)
	{
		printf("WSA Startup has failed.\n");
		return 1;
	}

	SOCKET udpSocket = create_socket();

	if(udpSocket == INVALID_SOCKET)
	{
		return 1;
	}

	socket_address_add(&server, port_no, ip_address);

	if(bind(udpSocket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Cannot bind socket: %d\n", WSAGetLastError());
		closesocket(udpSocket);
		WSACleanup();
		return 1;
	}

	int clientLen = sizeof(client);
	int recvLen;
	signal(SIGINT, stop_socket);

	PcmBuffer pcm_buffer;
	if(init_PCM_buffer(&pcm_buffer, BUFFER_SIZE / 2) != 0)
	{
		printf("Failed to initialise PCM buffer\n");
		closesocket(udpSocket);
		WSACleanup();
		return 1;
	}

	while(*keep_running)
	{
		recvLen = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client, &clientLen);

		if(recvLen == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if(error == WSAEWOULDBLOCK)
			{				
				continue;
			}
			else
			{
				printf("Error getting data from socket: %d\n", WSAGetLastError());
				break;
			}
		}

		rtp_filtering((uint8_t*)buffer, recvLen, &pcm_buffer, path);
		buffer[recvLen] = '\0';
		trigger_data_rec((const char*)buffer);
	}

	free_PCM_buffer(&pcm_buffer);
	closesocket(udpSocket);
	WSACleanup();
	printf("Socket closed.\n");

	return 1;
}