#ifndef AUDIO_FILE_CREATOR_H_
#define AUDIO_FILE_CREATOR_H_
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct wav_header;
void create_wav_file(FILE* file, int data_size);
void save_wav_file(int* pcm_buffer_len, int16_t* pcm_buffer);

#endif // !AUDIO_FILE_CREATOR_H_
