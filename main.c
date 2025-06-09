#include <stdlib.h>

#include "src/wavngn.h"

int main() {
	sample_t *pcm = NULL;
	int pcmPtr = 0;
	int numSamples = 0;

	AudioModifiers modifiers = {AF_SINE, {0.5, 0}, 0.2, 0.2, 1, 5, 0.5};

	AppendToneToPCM(&pcm, &pcmPtr, &numSamples, 440.0f, 1,
					modifiers);	 // A4, 1 sec

	pcmPtr += AppendToneToPCM(&pcm, &pcmPtr, &numSamples, 660.0f, 1,
							  modifiers);  // E5, 1 sec

	modifiers = (AudioModifiers){AF_SINE, {0, 0.5}, 0.2, 0.2, 1, 5, 0.5};

	AppendToneToPCM(&pcm, &pcmPtr, &numSamples, 440.0f, 1,
					modifiers);	 // A4, 1 sec

	pcmPtr += AppendToneToPCM(&pcm, &pcmPtr, &numSamples, 660.0f, 1,
							  modifiers);  // E5, 1 sec

	sample_t *new = NULL;
	int newPtr = 0;
	int newSamples = 0;
	AppendPCMToPCM(&new, &newPtr, &newSamples, pcm, numSamples);

	WriteWAVFromPCM("output.wav", new, newSamples);

	system("ffplay -nodisp -autoexit output.wav");

	free(pcm);
	free(new);
	return 0;
}
