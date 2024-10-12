#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "AudioFileCreator.h"


/// <summary>
/// Defines the structure of the header for a wav file
/// </summary>
struct wav_header
{
	char riff[CHAR_LENGTHS];
	int32_t file_len;
	char wave[CHAR_LENGTHS];
	char format[CHAR_LENGTHS];
	int32_t chunk_size;
	int16_t format_tag;
	int16_t channels;
	int32_t sample_rate;
	int32_t bytes_p_s;
	int16_t bytes_p_sam;
	int16_t bits_p_sam;
	char data[CHAR_LENGTHS];
	int32_t data_length;
};

/// <summary>
/// Creates a wave file with the given data size
/// additioanally creates all header data for the file
/// </summary>
/// <param name="file">File being created</param>
/// <param name="data_size">Size of the data being saved into this WAV</param>
void create_wav_file(FILE* file, int data_size)
{
	struct wav_header header;
	memcpy(header.riff, "RIFF", CHAR_LENGTHS);
	header.file_len = data_size + sizeof(header) - 8;
	memcpy(header.wave, "WAVE", CHAR_LENGTHS);
	memcpy(header.format, "fmt ", CHAR_LENGTHS);
	header.chunk_size = 16;
	header.format_tag = 1;
	header.channels = CHANNELS;
	header.sample_rate = SAMPLE_RATE;
	header.bytes_p_s = SAMPLE_RATE * 1 * (BITS_PER_SAMPLE / 8);
	header.bytes_p_sam = CHANNELS * (BITS_PER_SAMPLE / 8);
	header.bits_p_sam = BITS_PER_SAMPLE;
	memcpy(header.data, "data", CHAR_LENGTHS);
	header.data_length = data_size;

	fwrite(&header, sizeof(header), 1, file);
}

/// <summary>
/// Creates a wav file and saves the existing buffer data to the wav file
/// </summary>
/// <param name="pcm_buffer_len">Length of data being passed into the wav file</param>
/// <param name="pcm_buffer">Data being passed into the wav file</param>
void save_wav_file(int* pcm_buffer_len, int16_t* pcm_buffer)
{
	static int fileIndex = 0;
	char filename[50];
	snprintf(filename, sizeof(filename), "output_%d.wav", fileIndex++);

	FILE* wav_file = fopen(filename, "wb");
	if (!wav_file)
	{
		perror("Failed to open WAV file");
		return;
	}

	create_wav_file(wav_file, *pcm_buffer_len * sizeof(int16_t));

	if(fwrite(pcm_buffer, sizeof(int16_t), *pcm_buffer_len, wav_file) != *pcm_buffer_len)
	{
		perror("Failed to write audio data to WAV file");
		fclose(wav_file);
		return;
	}

	fclose(wav_file);
}