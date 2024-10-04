#include "SocketRec.h"
#include "SortingAlgorithms.h"

/// <summary>
/// Creates a socket that will be used to receive VoIP data.
/// </summary>
/// <returns>A UDP socket, integer value</returns>
SOCKET create_socket()
{
  SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if(udpSocket == INVALID_SOCKET)
  {
    printf("Error creating socket. Error is: %d\n", WSAGetLastError());
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
    printf("Cannot start up WSA. Error is: %d\n", WSAGetLastError());
    return 1;
  }

  return 0;
}

/// <summary>
/// Assigns address type, port and IP, used later to bind to the socket
/// </summary>
/// <param name="addr">Data structure containing connection details</param>
/// <param name="port">Port number to listen on</param>
void socket_address_add(struct sockaddr_in* addr, int port)
{
  memset(addr, 0, sizeof(*addr));
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  addr->sin_port = htons(port);
}

/// <summary>
/// Binds the socket to previously set data
/// </summary>
/// <param name="udpSocket">The socket being set</param>
/// <param name="server">The information being used to bind the socket</param>
void bind_socket(SOCKET udpSocket, struct sockaddr_in* server) 
{
  if(bind(udpSocket, (struct sockaddr*)server, sizeof(*server)) == SOCKET_ERROR)
  {
    printf("Cannot bind socket. Error is: %d\n", WSAGetLastError());
    closesocket(udpSocket);
    WSACleanup();
  }
}

/// <summary>
/// Creates a looping buffer to recieve packets via UDP
/// </summary>
/// <returns>0 or 1 based on performance</returns>
int start_listening()
{
  struct sockaddr_in server, client;
  char buffer[512];

  if(wsa_startup() == 1)
  {
    printf("WSA Startup has failed.");
  }

  SOCKET udpSocket = create_socket();

  if(udpSocket == INVALID_SOCKET)
  {
    return 1;
  }

  socket_address_add(&server, 514);

  bind_socket(udpSocket, &server);

  int clientLen = sizeof(client);
  int recvLen;

  while(1)
  {
    recvLen = recvfrom(udpSocket, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client, &clientLen);

    if(recvLen == SOCKET_ERROR)
    {
      printf("Error getting data from socket. Error is: %d\n", WSAGetLastError());
      break;
    }

    buffer[recvLen] = '\0';
  }

  closesocket(udpSocket);
  WSACleanup();
  printf("Socket closed and cleaned up.\n");

  return 1;
}