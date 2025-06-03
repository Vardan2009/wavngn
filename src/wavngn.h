#ifndef _WAVNGN_
#define _WAVNGN_

#include <stdint.h>

#include "definitions.h"

// RIFF Header
typedef struct {
	char chunkID[4];  // RIFF
	uint32_t chunkSize;
	char format[4];	 // WAVE
} RIFFHeader;

// fmt Subchunk
typedef struct {
	char subChunk1ID[4];	 // "fmt "
	uint32_t subChunk1Size;	 // 16 for PCM
	uint16_t audioFormat;	 // PCM = 1
	uint16_t numChannels;	 // Mono = 1, Stereo = 2
	uint32_t sampleRate;	 // 44100, 48000, etc.
	uint32_t byteRate;		 // = SampleRate * NumChannels * BitsPerSample / 8
	uint16_t blockAlign;	 // = NumChannels * BitsPerSample / 8
	uint16_t bitsPerSample;	 // 8, 16, 24, 32
} FmtSubchunk;

// data Subchunk
typedef struct {
	char subChunk2ID[4];	 // "data"
	uint32_t subChunk2Size;	 // = NumSamples * NumChannels * BitsPerSample / 8
							 // Followed by actual sound data
} DataSubchunk;

typedef struct {
	RIFFHeader riff;
	FmtSubchunk fmt;
	DataSubchunk data;
} WAVHeader;

typedef enum { AF_SINE, AF_SQUARE } audiofn_t;

typedef struct {
	audiofn_t function;
	double volume;

	double attack, decay, release;
} AudioModifiers;

int AppendToneToPCM(sample_t **pcmBuffer, int *pcmPtr, int *numSamples,
					float frequency, float durationSeconds,
					AudioModifiers mods);

void WriteWAVFromPCM(const char *filename, sample_t *pcmBuffer, int numSamples);

#endif	// _WAVNGN_
