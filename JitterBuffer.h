#ifndef JITTER_BUFFER_H
#define JITTER_BUFFER_H

#include <stdint.h>

#define SAMPLE_RATE 44100
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define DURATION 5
#define BUFFER_SIZE (SAMPLE_RATE * CHANNELS * (BITS_PER_SAMPLE / 8) * DURATION)

typedef struct
{
  int16_t* buffer;
  int length;
  int max_length;
} JitterBuffer;

void init_jitter_buffer(JitterBuffer* jb, int max_length);
void add_to_jitter_buffer(JitterBuffer* jb, int16_t* data, int data_length);
void process_jitter_buffer(JitterBuffer* jb);
void free_jitter_buffer(JitterBuffer* jb);

#endif // JITTER_BUFFER_H