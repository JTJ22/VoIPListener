#ifndef AUDIO_FILE_CREATOR_H_
#define AUDIO_FILE_CREATOR_H_
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define CHAR_LENGTHS 4
#define SAMPLE_RATE 44100
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define BYTES_IN_BUFFER 20480
#define BYTES_PCM 88200

struct wav_header;
void create_wav_file(FILE* file, int data_size);
void save_wav_file(int* pcm_buffer_len, int16_t* pcm_buffer);

#endif // !AUDIO_FILE_CREATOR_H_
