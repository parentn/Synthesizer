#ifndef ADD_SIGNAL_HPP
#define ADD_SIGNAL_HPP

#include "ofMain.h"

typedef struct s_signal{
	float phase;
	float frequency;
	float volume;
} s_signal;

void add_signal(ofSoundBuffer &buffer, s_signal signal);
#endif // ADD_SIGNAL_HPP