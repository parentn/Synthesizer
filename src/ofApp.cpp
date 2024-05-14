#include "ofApp.h"
#include <complex>
#include <math.h>
#include <iostream>

//--------------------------------------------------------------

void ofApp::addSignal(s_signal& signal){
	float pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

    float& phase = signal.phase;
    float volume = signal.volume;
    float freq = signal.frequency;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:

    for (size_t i = 0; i < bufferSize; i++){
		while (phase > TWO_PI){
			phase -= TWO_PI;
		}
        float sample = sin(phase);
        lAudio[i] += sample * volume * leftScale;
        rAudio[i] += sample * volume * rightScale;
        phase += 2.0 * M_PI * freq / ((float) sampleRate); // 2Pi * freq * dt;
    }
}

void ofApp::initSignal(){
	for (size_t i = 0; i < bufferSize; i++){
		lAudio[i] = 0.;
		rAudio[i] = 0.;
    }
}


void ofApp::setup(){

	ofBackground(34, 34, 34);

	bufferSize = soundStream.getBufferSize();
	sampleRate = soundStream.getSampleRate();
	// bufferSize		= 512;
	// sampleRate 		= 44100;

	std::cout << "Buffer Size is " << bufferSize << std::endl;
	std::cout << "Sample Rate is " << sampleRate << std::endl;
	
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;
	octaveIndex			= 4;
	mNote				= Notes::A;

	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	dftAudio.assign(bufferSize, 0.0);
	dftAudioNorm.assign(bufferSize, 0.0); 

	
	soundStream.printDeviceList();

	ofSoundStreamSettings settings;

	// To be removed as we want to trigger ourself the signals	
	signals.clear();
	s_signal signal(0., 1., 0.5);
	signals.push_back(signal);

	// if you want to set the device id to be different than the default:
	//
	//	auto devices = soundStream.getDeviceList();
	//	settings.setOutDevice(devices[3]);

	// you can also get devices for an specific api:
	//
	//	auto devices = soundStream.getDeviceList(ofSoundDevice::Api::PULSE);
	//	settings.setOutDevice(devices[0]);

	// or get the default device for an specific api:
	//
	// settings.api = ofSoundDevice::Api::PULSE;

	// or by name:
	//
	//	auto devices = soundStream.getMatchingDevices("default");
	//	if(!devices.empty()){
	//		settings.setOutDevice(devices[0]);
	//	}

#ifdef TARGET_LINUX
	// Latest linux versions default to the HDMI output
	// this usually fixes that. Also check the list of available
	// devices if sound doesn't work
	auto devices = soundStream.getMatchingDevices("default");
	if(!devices.empty()){
		settings.setOutDevice(devices[0]);
	}
#endif

	settings.setOutListener(this);
	settings.sampleRate = sampleRate;
	settings.numOutputChannels = 2;
	settings.numInputChannels = 0;
	settings.bufferSize = bufferSize;
	soundStream.setup(settings);

	// on OSX: if you want to use ofSoundPlayer together with ofSoundStream you need to synchronize buffersizes.
	// use ofFmodSetBuffersize(bufferSize) to set the buffersize in fmodx prior to loading a file.
}


//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
std::vector<std::complex<float>> compute_dft(std::vector<std::complex<float>> dftAudio, std::vector<float> audio){
	int size_sample = audio.size();
	std::complex<float> omega = std::exp(std::complex<float>(0, - 2. * M_PI / size_sample));
	int i = 0;
	std::complex<float> omega_i(1.0,0.0);
	int k = 0;
	std::complex<float> omega_i_k(1.0,0.0);
	for(auto &v : dftAudio){
		v = std::complex<float>(0., 0.);
		omega_i_k = std::complex<float>(1.0,0.0);
		for (auto &u : audio){
			v += u * omega_i_k;
			k++;
			omega_i_k *= omega_i;
		}
		v /= std::sqrt(size_sample);
		i++;
		omega_i *= omega;
	}

	return dftAudio;
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofSetColor(225);
	ofDrawBitmapString("AUDIO OUTPUT EXAMPLE", 32, 32);
	ofDrawBitmapString("press 'b' to unpause the audio\npress 'n' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 150, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Left Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			
			for (unsigned int i = 0; i < lAudio.size(); i++){
				float x =  ofMap(i, 0, lAudio.size(), 0, 900, true);
				ofVertex(x, 100 -lAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	// draw the right channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 350, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Right Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			
			for (unsigned int i = 0; i < rAudio.size(); i++){
				float x =  ofMap(i, 0, rAudio.size(), 0, 900, true);
				ofVertex(x, 100 -rAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();


	// draw the DFT:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 550, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("DFT of right signal", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);

		ofTranslate(0, 200, 0);
		dftAudio = compute_dft(dftAudio, rAudio);

		float maxDft = 0.0;
		for(size_t i=0; i < dftAudio.size(); i++){
			dftAudioNorm[i] = std::norm(dftAudio[i]);
			maxDft = (dftAudioNorm[i] > maxDft) ? dftAudioNorm[i] : maxDft;
		}
			// ofBeginShape();
			for (unsigned int i = 0; i < dftAudio.size(); i++){
				float x =  ofMap(i, 0, dftAudio.size(), 0, 900, true);
				float y = ofMap(std::norm(dftAudio[i]), 0, maxDft, 0, 200, true);
				// ofVertex(x, 100 -std::norm(dftAudio[i])*180.0f);
				ofDrawLine(x, float(0.), x, -y);
			}
			// ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();
	
		
	ofSetColor(225);
	string reportString = "volume: ("+ofToString(volume, 2)+") modify with -/+ keys\npan: ("+ofToString(pan, 2)+") modify with mouse x\nsynthesis: ";
	if( !bNoise ){
		reportString += "sine wave (" + ofToString(targetFrequency, 2) + "hz) modify with mouse y";
	}else{
		reportString += "noise";	
	};
	reportString += "\noctave: "+ofToString(octaveIndex, 2)+", modify with w/x keys";
	// reportString+= "\ncurrent note"+ofToString(mNote, 2);
	ofDrawBitmapString(reportString, 32, 779);

	// 	ofSetColor(225);
	// string reportString = "volume: ("+ofToString(volume, 2)+") modify with -/+ keys\npan: ("+ofToString(pan, 2)+") modify with mouse x\nsynthesis: ";
	// if( !bNoise ){
	// 	reportString += "sine wave (" + ofToString(targetFrequency, 2) + "hz) modify with mouse y";
	// }else{
	// 	reportString += "noise";	
	// }
	// ofDrawBitmapString(reportString, 32, 779);
}

//--------------------------------------------------------------
float ofApp::pitchToFrequency(int pitch, float A4frequency = 440.f, int A4pitch = 57){
	return A4frequency * pow(2, ((pitch - A4pitch) / 12.f));
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
	if (key == '-' || key == '_' ){
		volume -= 0.05;
		volume = MAX(volume, 0);
	} else if (key == '+' || key == '=' ){
		volume += 0.05;
		volume = MIN(volume, 1);
	}
	
	if( key == 'b' ){
		soundStream.start();
	}
	
	if( key == 'n' ){
			soundStream.stop();
		}

	switch (key)
	{
	case 'w':
		octaveIndex=octaveIndex-1;
		break;
	case 'x':
		octaveIndex=octaveIndex+1;
		break;
	default:
		break;
	}

	switch (key)
		{
		case 'q':
			mNote=Notes::C;	
			break;
		case 'z':
			mNote=Notes::Db;
			break;
		case 's':
			mNote=Notes::D;
			break;
		case 'e':
			mNote=Notes::Eb;
			break;
		case 'd':
			mNote=Notes::E;
			break;
		case 'f':
			mNote=Notes::F;
			break;
		case 't':
			mNote=Notes::Gb;
			break;
		case 'g':
			mNote=Notes::G;
			break;
		case 'y':
			mNote=Notes::Ab;
			break;
		case 'h':
			mNote=Notes::A;
			break;
		case 'u':
			mNote=Notes::Bb;
			break;
		case 'j':
			mNote=Notes::B;
			break;
		default:
			// compilation error: jump to default:
			mNote=Notes::A;
			break;
		}
	int pitchIndex = static_cast<int>(mNote);
	int pitch=pitchIndex+octaveIndex*12;
	float targetFrequency=pitchToFrequency(pitch); // initialization
	phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;

}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){

}

// remove the frequency change with moving mouse
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	// int width = ofGetWidth();
	// pan = (float)x / (float)width;
	// float height = (float)ofGetHeight();
	// float heightPct = ((height-y) / height);
	// targetFrequency = 2000.0f * heightPct;
	// targetFrequency = 3000.0f;
	// phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	// int width = ofGetWidth();
	// pan = (float)x / (float)width;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	bNoise = true;
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bNoise = false;
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer & buffer){
	initSignal();
	for(auto & signal : signals ){
		addSignal(signal);
	}
	for (size_t i = 0; i < buffer.getNumFrames(); i++){
		buffer[i*buffer.getNumChannels()    ] = lAudio[i]; // = sample * volume * leftScale;
		buffer[i*buffer.getNumChannels() + 1] = rAudio[i]; // = sample * volume * rightScale;
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
