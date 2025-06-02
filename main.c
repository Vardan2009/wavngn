#include <stdlib.h>

#include "src/wavngn.h"

int main() {
	sample_t *pcm = NULL;
	int numSamples = 0;

	AppendToneToPCM(&pcm, &numSamples, 440.0f, 1);	// A4, 1 sec
	AppendToneToPCM(&pcm, &numSamples, 660.0f, 1);	// E5, 1 sec

	WriteWAVFromPCM("output.wav", pcm, numSamples);

	free(pcm);
	return 0;
}
