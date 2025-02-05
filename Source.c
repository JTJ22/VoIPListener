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
} socket_params;

__declspec(dllexport) int main(const char* ip_address, int port_no)
{
	if(run_program(ip_address, port_no) != 0)
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

	start_listening(ip_address, port_no);
	free(params);
	return 0;
}

/// <summary>
/// Asks the user for a port number to listen on, then creates a thread to listen on that port
/// </summary>
/// <returns>0 once complete</returns>
int run_program(const char* ip_address, int port_no)
{
	socket_params* params = (socket_params*)malloc(sizeof(socket_params));
	if(params == NULL)
	{
		printf("Memory allocation failed\n");
		return 1;
	}

	params->ip_address = ip_address; 
	params->port_number = port_no;
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

	DWORD await = WaitForSingleObject(thread, INFINITE);
	if(await == WAIT_FAILED)
	{
		printf("Thread failed to execute: %d\n", GetLastError());
		CloseHandle(thread);
		free(params);
		return 1;
	}

	DWORD exit_code;
	if(!GetExitCodeThread(thread, &exit_code))
	{
		printf("Cannot get exit code: %d\n", GetLastError());
		CloseHandle(thread);
		free(params);
		return 1;

	}

	CloseHandle(thread);
	free(params);
	return 0;
}