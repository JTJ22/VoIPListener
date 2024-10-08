#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

struct wav_header
{
	char riff[4];
	int32_t file_len;
	char wave[4];
	char format[4];
	int32_t chunk_size;
	int16_t format_tag;
	int16_t channels;
	int32_t sample_rate;
	int32_t bytes_p_s;
	int16_t bytes_p_sam;
	int16_t bits_p_sam;
	char data[4];
	int32_t data_length;
};

void create_wav_file(FILE* file, int data_size)
{
  struct wav_header header;
  memcpy(header.riff, "RIFF", 4);
  header.file_len = data_size + sizeof(header) - 8;
  memcpy(header.wave, "WAVE", 4);
  memcpy(header.format, "fmt ", 4);
  header.chunk_size = 16;
  header.format_tag = 1;
  header.channels = 1;
  header.sample_rate = 44100;
  header.bytes_p_s = 44100 * 1 * (16 / 8);
  header.bytes_p_sam = 1 * (16 / 8);
  header.bits_p_sam = 16;
  memcpy(header.data, "data", 4);
  header.data_length = data_size;

  fwrite(&header, sizeof(header), 1, file);
}

void save_wav_file(int* pcm_buffer_len, int16_t* pcm_buffer, int* packet_count)
{
  static int fileIndex = 0;
  char filename[50];
  snprintf(filename, sizeof(filename), "output_%d.wav", fileIndex++);

  FILE* wav_file = fopen(filename, "wb");
  if(!wav_file) 
  {
    perror("Failed to open WAV file");
    return;
  }

  create_wav_file(wav_file, *pcm_buffer_len * sizeof(int16_t));

  fwrite(pcm_buffer, sizeof(int16_t), *pcm_buffer_len, wav_file);

  fclose(wav_file);

  printf("WAV file created: %s with %d bytes of audio data.\n", filename, *pcm_buffer_len);

  *pcm_buffer_len = 0;
  *packet_count = 0;
}