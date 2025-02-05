#ifndef PCM_BUFFER_H
#define PCM_BUFFER_H

#include <stdint.h>
#include "AudioFileCreator.h"
#include "GlobalVariables.h"

typedef struct
{
  int16_t* buffer;
  int length;
  int max_length;
} PcmBuffer;

int init_PCM_buffer(PcmBuffer* pcb, int max_length);
void add_to_PCM_buffer(PcmBuffer* pcb, int16_t* data, int data_length);
void process_PCM_buffer(PcmBuffer* pcb);
void free_PCM_buffer(PcmBuffer* pcb);

#endif // PCM_BUFFER_H