#include "wavngn.h"

#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "definitions.h"

void AppendToneToPCM(sample_t **pcmBuffer, int *numSamples, float frequency,
					 int durationSeconds, AudioModifiers mods) {
	int samplesToAdd = durationSeconds * _CFG_SAMPLE_RATE;

	if (samplesToAdd < 0) {
		fprintf(stderr, "Duration in seconds must be non-negative\n");
		exit(1);
	}

	if (samplesToAdd < 0 || durationSeconds > INT_MAX / _CFG_SAMPLE_RATE) {
		fprintf(stderr,
				"Arithmetic overflow detected in durationSeconds * "
				"_CFG_SAMPLE_RATE\n");
		exit(1);
	}

	int newNumSamples = *numSamples + samplesToAdd;
	if (newNumSamples < *numSamples) {
		fprintf(stderr,
				"Arithmetic overflow detected in *numSamples + samplesToAdd\n");
		exit(1);
	}

	sample_t *newBuffer =
		realloc(*pcmBuffer, newNumSamples * 2 * sizeof(sample_t));
	if (!newBuffer) {
		perror("Failed to allocate PCM buffer");
		free(*pcmBuffer);
		exit(1);
	}
	*pcmBuffer = newBuffer;

	for (int i = 0; i < samplesToAdd; i++) {
		double t = (double)(*numSamples + i) / _CFG_SAMPLE_RATE;
		double arg = 2 * PI * frequency * t;
		double amplitude = 0;
		switch (mods.function) {
			case AF_SINE:
				amplitude = sin(arg);
				break;
			case AF_SQUARE:
				amplitude = sin(arg) > 0 ? 1 : -1;
				break;
			default:
				fprintf(stderr, "Unknown audio function\n");
				exit(1);
		}
		amplitude *= mods.volume;
		sample_t sample = (sample_t)(amplitude * _CFG_SAMPLE_MAX);

		for (int j = 0; j < _CFG_CHANNELS; ++j)
			(*pcmBuffer)[(*numSamples + i) * _CFG_CHANNELS + j] = sample;
	}

	*numSamples = newNumSamples;
}

void WriteWAVFromPCM(const char *filename, sample_t *pcmBuffer,
					 int numSamples) {
	static const int byteRate =
		_CFG_SAMPLE_RATE * _CFG_CHANNELS * _CFG_SAMPLE_BITS / 8;
	static const int blockAlign = _CFG_CHANNELS * _CFG_SAMPLE_BITS / 8;
	int dataSize = numSamples * blockAlign;

	WAVHeader header;

	memcpy(header.riff.chunkID, "RIFF", 4);
	header.riff.chunkSize = 36 + dataSize;
	memcpy(header.riff.format, "WAVE", 4);

	memcpy(header.fmt.subChunk1ID, "fmt ", 4);
	header.fmt.subChunk1Size = 16;
	header.fmt.audioFormat = 1;
	header.fmt.numChannels = _CFG_CHANNELS;
	header.fmt.sampleRate = _CFG_SAMPLE_RATE;
	header.fmt.byteRate = byteRate;
	header.fmt.blockAlign = blockAlign;
	header.fmt.bitsPerSample = _CFG_SAMPLE_BITS;

	memcpy(header.data.subChunk2ID, "data", 4);
	header.data.subChunk2Size = dataSize;

	FILE *file = fopen(filename, "wb");
	if (!file) {
		perror("Failed to open file for writing");
		return;
	}

	fwrite(&header, sizeof(WAVHeader), 1, file);
	fwrite(pcmBuffer, sizeof(sample_t), numSamples * _CFG_CHANNELS, file);
	fclose(file);
}
