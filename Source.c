#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include "SocketRec.h"

int main()
{
	start_listening(514);
	return 0;
}