#include "PcmBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/// <summary>
/// Initialises the PCM buffer to store recieved packets
/// </summary>
/// <param name="pcb">Current buffer being initialised</param>
/// <param name="max_length">The max length of the buffer</param>
int init_PCM_buffer(PcmBuffer* pcb, int max_length)
{
	pcb->buffer = (int16_t*)malloc(max_length * sizeof(int16_t));
	if(!pcb->buffer)
	{
		perror("Failed malloc for PCM Buffer");
		exit(EXIT_FAILURE);
		return 1;
	}
	pcb->length = 0;
	pcb->max_length = max_length;
	return 0;
}

/// <summary>
/// Adds data to the PCM buffer if there is space to add the data
/// </summary>
/// <param name="pcb">The buffer having the data added</param>
/// <param name="data">The data being added to the buffer</param>
/// <param name="data_length">The total length of the data being added to the buffer</param>
void add_to_PCM_buffer(PcmBuffer* pcb, int16_t* data, int data_length)
{
	if(pcb->length + data_length > pcb->max_length)
	{
		printf("PCM buffer overflown\n");
		return;
	}
	memcpy(pcb->buffer + pcb->length, data, data_length * sizeof(int16_t));
	pcb->length += data_length;
}

/// <summary>
/// If the buffer has reached the max length, save the buffer to a wav file
/// </summary>
/// <param name="pcb">The buffer being checked</param>
void process_PCM_buffer(PcmBuffer* pcb, const char* path)
{
	save_wav_file(path, &pcb->length, pcb->buffer);
	pcb->length = 0;
	memset(pcb->buffer, 0, BYTES_PCM / 2);
}

/// <summary>
/// Frees the memory allocated for the jitter buffer
/// </summary>
/// <param name="jb">The buffer having memory freed</param>
void free_PCM_buffer(PcmBuffer* pcb)
{
	free(pcb->buffer);
}