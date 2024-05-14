#pragma once

#include "ofMain.h"
#include <complex>
typedef struct{
	float phase;
	float frequency;
	float volume;
} s_signal;

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		
		void audioOut(ofSoundBuffer & buffer);
		
		ofSoundStream soundStream;

		float 	pan;
		// int	sampleRate;
		bool 	bNoise;
		float 	volume;

		vector <float> lAudio;
		vector <float> rAudio;

		ofSoundBuffer buffer;
		
		//------------------- for the simple sine wave synthesis
		float 	targetFrequency;
		float 	phase;
		float 	phaseAdder;
		float 	phaseAdderTarget;

		void addSignal(s_signal signal);
		void initSignal();
		size_t bufferSize;
		size_t sampleRate;
		vector <std::complex<float>> dftAudio;
		vector<float> dftAudioNorm;
		vector<s_signal> signals;
};
