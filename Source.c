#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <windows.h>
#include "SocketRec.h"

int main()
{
	run_program();
	return 0;
}

/// <summary>
/// On a new thread starts listening for incoming VoIP data
/// </summary>
/// <param name="lpParam">Parameter for the thread which is the port number</param>
/// <returns>0 indicating function has complete</returns>
DWORD WINAPI listening_thread(LPVOID lpParam)
{
	int port_no = *(int*)lpParam;
	start_listening(port_no);
	return 0;
}

/// <summary>
/// Asks the user for a port number to listen on, then creates a thread to listen on that port
/// </summary>
/// <returns>0 once complete</returns>
int run_program()
{
	int port_no;
	printf("Enter the port number to listen on: ");
	scanf_s("%d", &port_no);

	return create_thread(port_no);
}

/// <summary>
/// Creates a new thread and waits for it to complete, checks for errors. 
/// </summary>
/// <param name="port_no">The port number to listen on</param>
/// <returns>0 when complete, 1 with an error</returns>
int create_thread(int port_no)
{
	HANDLE thread = CreateThread(NULL, 0, listening_thread, &port_no, 0, NULL);
	if(thread == NULL)
	{
		printf("Cannot create thread: %d\n", GetLastError());
		return 1;
	}

	DWORD await = WaitForSingleObject(thread, INFINITE);
	if(await == WAIT_FAILED)
	{
		printf("Thread failed to execute: %d\n", GetLastError());
		CloseHandle(thread);
		return 1;
	}

	DWORD exit_code;
	if(!GetExitCodeThread(thread, &exit_code))
	{
		printf("Cannot get exit code: %d\n", GetLastError());
		CloseHandle(thread);
		return 1;

	}

	CloseHandle(thread);
	return 0;
}