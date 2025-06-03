#include <stdlib.h>

#include "src/wavngn.h"

int main() {
	sample_t *pcm = NULL;
	int pcmPtr = 0;
	int numSamples = 0;

	AudioModifiers modifiers = {AF_SINE, 0.5, 0.5, 0.2, 1};

	AppendToneToPCM(&pcm, &pcmPtr, &numSamples, 440.0f, 1,
					modifiers);	 // A4, 1 sec

	pcmPtr += AppendToneToPCM(&pcm, &pcmPtr, &numSamples, 660.0f, 1,
							  modifiers);  // E5, 1 sec

	WriteWAVFromPCM("output.wav", pcm, numSamples);

	system("ffplay -nodisp -autoexit output.wav");

	free(pcm);
	return 0;
}
