#include "JitterBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "AudioFileCreator.h"

/// <summary>
/// Initialises the jitter buffer to store recieved packets
/// </summary>
/// <param name="jb">Current buffer being initialised</param>
/// <param name="max_length">The max length of the buffer</param>
void init_jitter_buffer(JitterBuffer* jb, int max_length)
{
	jb->buffer = (int16_t*)malloc(max_length * sizeof(int16_t));
	if (!jb->buffer)
	{
		perror("Failed to allocate memory for jitter buffer");
		exit(EXIT_FAILURE);
	}
	jb->length = 0;
	jb->max_length = max_length;
}

/// <summary>
/// Adds data to the jitter buffer if there is space to add the data
/// </summary>
/// <param name="jb">The buffer having the data added</param>
/// <param name="data">The data being added to the buffer</param>
/// <param name="data_length">The total length of the data being added to the buffer</param>
void add_to_jitter_buffer(JitterBuffer* jb, int16_t* data, int data_length)
{
	if (jb->length + data_length > jb->max_length)
	{
		printf("Jitter buffer overflow\n");
		return;
	}
	memcpy(jb->buffer + jb->length, data, data_length * sizeof(int16_t));
	jb->length += data_length;
}

/// <summary>
/// If the buffer has reached the max length, save the buffer to a wav file
/// </summary>
/// <param name="jb">The buffer being checked</param>
void process_jitter_buffer(JitterBuffer* jb)
{
	if (jb->length >= jb->max_length)
	{
		save_wav_file(&jb->length, jb->buffer);
		jb->length = 0;
	}
}

/// <summary>
/// Frees the memory allocated for the jitter buffer
/// </summary>
/// <param name="jb">The buffer having memory freed</param>
void free_jitter_buffer(JitterBuffer* jb)
{
	free(jb->buffer);
}