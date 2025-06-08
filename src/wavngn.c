#include "wavngn.h"

#include <limits.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "definitions.h"

sample_t mixSamples(sample_t x, sample_t y) {
	double x_d = (double)x / 32768.0;
	double y_d = (double)y / 32768.0;
	double mixed = x_d + y_d;

	if (mixed > 1.0)
		mixed = 1.0;
	else if (mixed < -1.0)
		mixed = -1.0;

	return (sample_t)round(mixed * 32767.0);
}

int AppendToneToPCM(sample_t **pcmBuffer, int *pcmPtr, int *numSamples,
					float frequency, float durationSeconds,
					AudioModifiers mods) {
	int samplesToAdd = (durationSeconds + mods.release) * _CFG_SAMPLE_RATE;
	int samplesToAddToPtr = durationSeconds * _CFG_SAMPLE_RATE;

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

	int deltaSamples = *pcmPtr + samplesToAdd - *numSamples;

	int newNumSamples = *numSamples + deltaSamples;
	if (newNumSamples < *numSamples) {
		fprintf(stderr,
				"Arithmetic overflow detected in *numSamples + deltaSamples\n");
		exit(1);
	}

	sample_t *newBuffer =
		realloc(*pcmBuffer, newNumSamples * _CFG_CHANNELS * sizeof(sample_t));

	if (newBuffer && newNumSamples > *numSamples) {
		size_t oldSize = *numSamples * 2 * sizeof(sample_t);
		size_t newSize = newNumSamples * 2 * sizeof(sample_t);
		memset((char *)newBuffer + oldSize, 0, newSize - oldSize);
	}

	if (!newBuffer) {
		perror("Failed to allocate PCM buffer");
		free(*pcmBuffer);
		exit(1);
	}
	*pcmBuffer = newBuffer;

	for (int i = 0; i < samplesToAdd; i++) {
		double t = (double)(*pcmPtr + i) / _CFG_SAMPLE_RATE;

		double env = 1.0;
		double sampleTime = (double)i / _CFG_SAMPLE_RATE;
		if (mods.attack > 0 && sampleTime < mods.attack) {
			env = sampleTime / mods.attack;
		} else if (mods.decay > 0 && sampleTime < (mods.attack + mods.decay) &&
				   sampleTime >= mods.attack) {
			double decayTime = sampleTime - mods.attack;
			env = 1.0 - (decayTime / mods.decay) * (1.0 - 0.7);
		} else if (mods.decay > 0 && sampleTime >= (mods.attack + mods.decay) &&
				   sampleTime < (durationSeconds)) {
			env = 0.7;
		} else if (mods.release > 0 && sampleTime >= durationSeconds) {
			double releaseTime = sampleTime - durationSeconds;
			if (releaseTime < mods.release) {
				env = 0.7 * (1.0 - (releaseTime / mods.release));
			} else {
				env = 0.0;
			}
		}

		double arg =
			2 * PI * frequency * t +
			mods.vibratoAmplitude * sin(2 * PI * mods.vibratoFrequency * t);

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

		amplitude *= env;

		for (int j = 0; j < _CFG_CHANNELS; ++j) {
			sample_t sample = (sample_t)(amplitude * mods.channelVolumes[j] *
										 _CFG_SAMPLE_MAX);

			sample_t *dest = &(*pcmBuffer)[(*pcmPtr + i) * _CFG_CHANNELS + j];
			*dest = mixSamples(sample, *dest);
		}
	}

	*numSamples = newNumSamples;
	return samplesToAddToPtr;
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
