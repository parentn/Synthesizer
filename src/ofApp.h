#include "ofMain.h"
#include <complex>

typedef struct{
	float phase;
	float frequency;
	float volume;
} s_signal;

enum class Notes
{
	C,
	Db,
	D,
	Eb,
	E,
	F,
	Gb,
	G,
	Ab,
	A,
	Bb,
	B, 
	sizeNotes
};



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

		void addSignal_sin(s_signal& signal);
		void addSignal_saw(s_signal& signal);
		void addSignal_square(s_signal& signal);
		void initSignal();
		size_t bufferSize;
		size_t sampleRate;
		vector <std::complex<float>> dftAudio;
		vector<float> dftAudioNorm;
		vector<s_signal> signals;
		float 	pitchToFrequency(int pitch, float A4frequency, int A4pitch);
		Notes 	mNote;
		int 	octaveIndex;

		int 	mBrillance;
		static constexpr int numNotes = static_cast<int>(Notes::sizeNotes);
		s_signal signalsNotes[numNotes];
		s_signal singleNote;
};