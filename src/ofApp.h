#include "ofMain.h"
#include <complex>

typedef struct{
	float phase;
	float frequency;
	float volume;
} s_signal;

typedef struct{
	float x_1;
	float x_2;
	float y_1;
	float y_2;
} s_previous_values;

typedef struct{
	float b_0;
	float b_1;
	float b_2;
	float a_1;
	float a_2;
} s_filter;

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

		vector<float> lAudioFiltered;
		vector<float> rAudioFiltered;
		s_previous_values lAudioPreviousValues;
		s_previous_values rAudioPreviousValues;

		ofSoundBuffer buffer;
		
		//------------------- for the simple sine wave synthesis
		float 	targetFrequency;
		float 	phase;
		float 	phaseAdder;
		float 	phaseAdderTarget;

		void addSignal(s_signal& signal);
		void initSignal();
		void synthesizeSquaredSignal(float frequency, int brillance);
		void synthesizeSawToothSignal(float frequency, int brillance);
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

		float lowFrequency;
		float highFrequency;
		float lowQ;
		float highQ;
		s_filter lowFilter;
		s_filter highFilter;
		s_filter lowPassFilter(float frequency, float Q);
		s_filter highPassFilter(float frequency, float Q);
		void applyFilter(s_filter filter);
};