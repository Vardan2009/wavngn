#include "wavngn.h"

#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "definitions.h"

void AppendToneToPCM(sample_t **pcmBuffer, int *numSamples, float frequency,
					 int durationSeconds) {
	int samplesToAdd = durationSeconds * _CFG_SAMPLE_RATE;
	int newNumSamples = *numSamples + samplesToAdd;
	*pcmBuffer = realloc(*pcmBuffer, newNumSamples * 2 * sizeof(sample_t));

	if (!*pcmBuffer) {
		perror("Failed to allocate PCM buffer");
		exit(1);
	}

	for (int i = 0; i < samplesToAdd; i++) {
		double t = (double)(*numSamples + i) / _CFG_SAMPLE_RATE;
		double amplitude = sin(2 * PI * frequency * t);
		int16_t sample = (sample_t)(amplitude * _CFG_SAMPLE_MAX);

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
