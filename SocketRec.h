#ifndef SOCKETREC_H_
#define SOCKETREC_H_

#include "SocketRec.h"
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include "DecodeRtp.h"
#include "PcmBuffer.h"

#pragma comment(lib, "Ws2_32.lib")

typedef void (*CallbackFunction)(const char* message);
__declspec(dllexport) void event_register(CallbackFunction callback);
void trigger_data_rec(const char* message);
int start_listening(int* port_no);
SOCKET create_socket();
int wsa_startup();
void socket_address_add(struct sockaddr_in* addr, int port);

#endif // SOCKETREC_H_
