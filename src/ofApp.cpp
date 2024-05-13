#include "ofApp.h"
#include <complex>
#include <math.h>
#include "addSignal.h"

extern int bufferSize;
extern int sampleRate;
//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(34, 34, 34);
	
	// int bufferSize		= 512;
	//sampleRate 			= 44100;
	phase 				= 0;
	phaseAdder 			= 0.0f;
	phaseAdderTarget 	= 0.0f;
	volume				= 0.1f;
	bNoise 				= false;

	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	dftAudio.assign(bufferSize, 0.0);
	dftAudioNorm.assign(bufferSize, 0.0); 

	
	soundStream.printDeviceList();

	ofSoundStreamSettings settings;

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
	ofDrawBitmapString("press 's' to unpause the audio\npress 'e' to pause the audio", 31, 92);
	
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
	}
	ofDrawBitmapString(reportString, 32, 779);

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
	
	if( key == 's' ){
		soundStream.start();
	}
	
	if( key == 'e' ){
		soundStream.stop();
	}
	
}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	int width = ofGetWidth();
	pan = (float)x / (float)width;
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	targetFrequency = 2000.0f * heightPct;
	phaseAdderTarget = (targetFrequency / (float) sampleRate) * TWO_PI;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	int width = ofGetWidth();
	pan = (float)x / (float)width;
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
	//pan = 0.5f;
	float leftScale = 1 - pan;
	float rightScale = pan;

	// sin (n) seems to have trouble when n is very large, so we
	// keep phase in the range of 0-TWO_PI like this:
	while (phase > TWO_PI){
		phase -= TWO_PI;
	}
	if ( bNoise == true){
		// ---------------------- noise --------------
		for (size_t i = 0; i < buffer.getNumFrames(); i++){
			lAudio[i] = buffer[i*buffer.getNumChannels()    ] = ofRandom(0, 1) * volume * leftScale;
			rAudio[i] = buffer[i*buffer.getNumChannels() + 1] = ofRandom(0, 1) * volume * rightScale;
		}
	} else {
		// phaseAdder = 0.95f * phaseAdder + 0.05f * phaseAdderTarget;
		phaseAdder = phaseAdder + 2.0 * M_PI * 1000.0 * bufferSize / sampleRate;
		s_signal signal(phaseAdder, 1000.0, 0.5);
		add_signal(buffer, signal);
		for (size_t i = 0; i < buffer.getNumFrames(); i++){
			// phase += phaseAdder;
			// float sample = sin(phase);
			lAudio[i] = buffer[i*buffer.getNumChannels()    ]; // = sample * volume * leftScale;
			rAudio[i] = buffer[i*buffer.getNumChannels() + 1]; // = sample * volume * rightScale;
		}
	}


}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
