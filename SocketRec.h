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

int start_listening(int* port_no);
SOCKET create_socket();
int wsa_startup();
void socket_address_add(struct sockaddr_in* addr, int port);

#endif // SOCKET_REC_H_
