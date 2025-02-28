#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include "SocketRec.h"

#pragma comment(lib, "ws2_32.lib")

typedef struct
{
	int port_number;
	const char* ip_address;
	volatile bool keep_running;
	HANDLE thread_handle;
} socket_params;

__declspec(dllexport) int main(socket_params* params)
{
	if(run_program(params) != 0)
	{
		printf("Program did not run!");
	}
	return 0;
}

/// <summary>
/// On a new thread starts listening for incoming VoIP data
/// </summary>
/// <param name="lpParam">Parameter for the thread which is the port number</param>
/// <returns>0 indicating function has complete</returns>
DWORD WINAPI listening_thread(LPVOID lpParam)
{
	socket_params* params = (socket_params*)lpParam;
	const char* ip_address = params->ip_address;
	int port_no = params->port_number;

	start_listening(ip_address, port_no, &params->keep_running);
	return 0;
}

/// <summary>
/// Asks the user for a port number to listen on, then creates a thread to listen on that port
/// </summary>
/// <returns>0 once complete</returns>
int run_program(socket_params* params)
{
	params->keep_running = true;
	return create_thread(params);
}

/// <summary>
/// Creates a new thread and waits for it to complete, checks for errors. 
/// </summary>
/// <param name="port_no">The port number to listen on</param>
/// <returns>0 when complete, 1 with an error</returns>
int create_thread(socket_params* params)
{
	HANDLE thread = CreateThread(NULL, 0, listening_thread, params, 0, NULL);
	if(thread == NULL)
	{
		printf("Cannot create thread: %d\n", GetLastError());
		free(params);
		return 1;
	}
	params->thread_handle = thread;
	return 0;
}


__declspec(dllexport) void stop_listener(socket_params* params)
{
	if(params != NULL)
	{
		params->keep_running = false;
		if(params->thread_handle != NULL)
		{
			DWORD waitResult = WaitForSingleObject(params->thread_handle, INFINITE);
			if(waitResult == WAIT_OBJECT_0)
			{
				printf("Thread finished execution.\n");
			}
			else
			{
				printf("WaitForSingleObject failed: %d\n", GetLastError());
			}
			BOOL closeResult = CloseHandle(params->thread_handle);
			if(!closeResult)
			{
				printf("CloseHandle failed: %d\n", GetLastError());
			}
			else
			{
				printf("Handle closed successfully.\n");
			}
			params->thread_handle = NULL;
		}
	}
}