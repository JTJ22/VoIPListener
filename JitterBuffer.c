#include "JitterBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "AudioFileCreator.h"

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

void process_jitter_buffer(JitterBuffer* jb)
{
	if (jb->length >= jb->max_length)
	{
		save_wav_file(&jb->length, jb->buffer);
		jb->length = 0;
	}
}

void free_jitter_buffer(JitterBuffer* jb)
{
	free(jb->buffer);
}