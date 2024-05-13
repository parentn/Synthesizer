#include "addSignal.h"
#include <cmath>

extern int bufferSize;
extern int sampleRate;

void add_signal(ofSoundBuffer &buffer, s_signal signal){
	float pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

    float phase = signal.phase;
    float volume = signal.volume;
    float freq = signal.frequency;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
    for (size_t i = 0; i < buffer.getNumFrames(); i++){
        float value = phase + 2.0 * i * M_PI * freq / ((float) sampleRate);
        float sample = sin(value);
        buffer[i*buffer.getNumChannels()    ] = sample * volume * leftScale;
        buffer[i*buffer.getNumChannels() + 1] = sample * volume * rightScale;
    }
}