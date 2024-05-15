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

void ofApp::synthesizeSquaredSignal(float frequency, int brillance){
	for(int k=0; k<brillance; k++){
		s_signal signal(0., (float(2*k+1)*frequency), volume /((float)(2*k+1)));
		signals.push_back(signal);
	}
}

void ofApp::synthesizeSawToothSignal(float frequency, int brillance){
	float sign = 1.;
	for(int k=0; k<brillance; k++){
		s_signal signal(0., (float(k+1)*frequency), sign * volume /((float)(k+1)));
		signals.push_back(signal);
		sign = -sign;
	}
}

s_filter ofApp::lowPassFilter(float frequency, float Q){
	float omega_0 = TWO_PI * frequency / sampleRate;
	float cos_omega_0 = cos(omega_0);
	float alpha = sin(omega_0) / (2.0f * Q);
	s_filter filter;
	float a_0 = 1.0+alpha;
	filter.a_1 = (-2. * cos_omega_0) / a_0;
	filter.a_2 = (1.0 - alpha) / a_0;
	filter.b_1 = (1.0 - cos_omega_0) / (a_0);
	filter.b_0 =  filter.b_1 / 2.0;
	filter.b_2 = filter.b_0;
	return filter;
}

s_filter ofApp::highPassFilter(float frequency, float Q){
	float omega_0 = TWO_PI * frequency / sampleRate;
	float cos_omega_0 = cos(omega_0);
	float alpha = sin(omega_0) / (2.0f * Q);
	s_filter filter;
	float a_0 = 1.0+alpha;
	filter.a_1 = (-2. * cos_omega_0) / a_0;
	filter.a_2 = (1.0 - alpha) / a_0;
	filter.b_1 = -(1.0 + cos_omega_0) / (a_0);
	filter.b_0 = - filter.b_1 / 2.0;
	filter.b_2 = filter.b_0;
	return filter;
}

void ofApp::applyFilter(s_filter filter){
	for(size_t i=0; i < bufferSize; i++){
		lAudioFiltered[i] = filter.b_0 * lAudio[i] \ 
			+ filter.b_1 * lAudioPreviousValues.x_1 \
			+ filter.b_2 * lAudioPreviousValues.x_2 \
			- filter.a_1 * lAudioPreviousValues.y_1 \
			- filter.a_2 * lAudioPreviousValues.y_2;
		lAudioPreviousValues.x_2 = lAudioPreviousValues.x_1;
		lAudioPreviousValues.x_1 = lAudio[i];
		lAudioPreviousValues.y_2 = lAudioPreviousValues.y_1;
		lAudioPreviousValues.y_1 = lAudioFiltered[i];

		rAudioFiltered[i] = filter.b_0 * rAudio[i] \ 
			+ filter.b_1 * rAudioPreviousValues.x_1 \
			+ filter.b_2 * rAudioPreviousValues.x_2 \
			- filter.a_1 * rAudioPreviousValues.y_1 \
			- filter.a_2 * rAudioPreviousValues.y_2;
		rAudioPreviousValues.x_2 = rAudioPreviousValues.x_1;
		rAudioPreviousValues.x_1 = rAudio[i]; 
		rAudioPreviousValues.y_2 = rAudioPreviousValues.y_1;
		rAudioPreviousValues.y_1 = rAudioFiltered[i];
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
	mBrillance			= 0;

	lAudio.assign(bufferSize, 0.0);
	rAudio.assign(bufferSize, 0.0);
	dftAudio.assign(bufferSize, 0.0);
	dftAudioNorm.assign(bufferSize, 0.0); 
	
	soundStream.printDeviceList();

	ofSoundStreamSettings settings;

	// To be removed as we want to trigger ourself the signals	
	// signals.clear();
	// s_signal signal(0., 440., 0.1);
	// signals.push_back(signal);

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
	targetFrequency = 0.;
	singleNote = s_signal(0.,0.,0.2);

	for(auto & signal: signalsNotes){
		signal.phase = 0.;
		signal.frequency = 1.f;
		signal.volume = 0.0f;
	}

	// Filtering 
	rAudioFiltered.assign(bufferSize, 0.0);
	lAudioFiltered.assign(bufferSize, 0.0);
	lAudioPreviousValues = s_previous_values(0.f,0.f,0.f,0.f);
	rAudioPreviousValues = s_previous_values(0.f,0.f,0.f,0.f);
	lowFrequency = 500;
	highFrequency = 10000;
	lowQ = 0.1;
	highQ = 0.1;
	lowFilter = lowPassFilter(lowFrequency, lowQ);
	highFilter = highPassFilter(highFrequency, highQ);
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
//--------------------------------------------------------------
void ofApp::draw(){

ofSetColor(225);
	ofDrawBitmapString("AUDIO OUTPUT EXAMPLE", 32, 32);
	ofDrawBitmapString("press 's' to unpause the audio\npress 'e' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left and right channels:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 150, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Left Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 450, 200);

		ofSetColor(58, 135, 245);
		ofSetLineWidth(3);
					
			ofBeginShape();
			
			for (unsigned int i = 0; i < lAudio.size(); i++){
				float x =  ofMap(i, 0, lAudio.size(), 0, 450, true);
				ofVertex(x, 100 -lAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(32, 150, 0);
		ofTranslate(450,0,0);
			
		ofSetColor(225);
		ofDrawBitmapString("Right Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 450, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			
			for (unsigned int i = 0; i < rAudio.size(); i++){
				float x =  ofMap(i, 0, rAudio.size(), 0, 450, true);
				ofVertex(x, 100 -rAudio[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	// draw the left and right filtered signals:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 350, 0);
			
		ofSetColor(225);
		string info = "Left filtered at frequency " + ofToString(lowFrequency) + " with quality " + ofToString(lowQ,2); 
		ofDrawBitmapString(info, 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 450, 200);

		ofSetColor(58, 135, 245);
		ofSetLineWidth(3);
					
			ofBeginShape();
			
			for (unsigned int i = 0; i < lAudioFiltered.size(); i++){
				float x =  ofMap(i, 0, lAudioFiltered.size(), 0, 450, true);
				ofVertex(x, 100 -lAudioFiltered[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();

		ofPushMatrix();
		ofTranslate(32, 350, 0);
		ofTranslate(450,0,0);
			
		ofSetColor(225);
		ofDrawBitmapString("Right filtered", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 450, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			
			for (unsigned int i = 0; i < rAudioFiltered.size(); i++){
				float x =  ofMap(i, 0, rAudioFiltered.size(), 0, 450, true);
				ofVertex(x, 100 -rAudioFiltered[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	// draw the DFT:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 550, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("DFT Right : Red", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
		ofTranslate(0, 200, 0);
		float maxDft = 0.0;
		for(size_t i=0; i < dftAudio.size(); i++){
			dftAudioNorm[i] = std::norm(dftAudio[i]);
			maxDft = (dftAudioNorm[i] > maxDft) ? dftAudioNorm[i] : maxDft;
		}
		dftAudio = compute_dft(dftAudio, rAudio);
			ofBeginShape();
			for (unsigned int i = 0; i < dftAudio.size() / 2; i++){ // Display only half of the DFT
				// float freq = ofMap(i, 0, dftAudio.size() / 2, 2, 2000); // Map index to frequency range
				// float x =  ofMap(freq, 2, 2000, 0, 900, true); // Map frequency to x-axis
				float x =  ofMap(i, 0, dftAudio.size() / 2, 0, 900, true);
				float y = ofMap(std::norm(dftAudio[i]), 0, maxDft, 0, 200, true);
				ofVertex(x, -y);
			}
			// ofBeginShape();
			// for (unsigned int i = 0; i < dftAudio.size(); i++){
			// 	float x =  ofMap(i, 0, dftAudio.size(), 0, 900, true);
			// 	// ofVertex(x, 100 -std::norm(dftAudio[i])*180.0f);
			// 	ofDrawLine(x, float(0.), x, -y);
			// }
			// ofEndShape(false);
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 550, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("\nDFT Left : Blue", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 900, 200);

		ofSetColor(58, 135, 245); // Change colour to blue
		ofSetLineWidth(3);

		ofTranslate(0, 200, 0);
		dftAudio = compute_dft(dftAudio, rAudio);
		maxDft = 0.0;
		for(size_t i=0; i < dftAudio.size(); i++){
			dftAudioNorm[i] = std::norm(dftAudio[i]);
			maxDft = (dftAudioNorm[i] > maxDft) ? dftAudioNorm[i] : maxDft;
		}
			ofBeginShape();
			for (unsigned int i = 0; i < dftAudio.size() / 2; i++){ // Display only half of the DFT
				// float freq = ofMap(i, 0, dftAudio.size() / 2, 2, 2000); // Map index to frequency range
				// float x =  ofMap(freq, 2, 2000, 0, 900, true); // Map frequency to x-axis
				float x =  ofMap(i, 0, dftAudio.size() / 2, 0, 900, true);
				float y = ofMap(std::norm(dftAudio[i]), 0, maxDft, 0, 200, true);
				ofVertex(x, -y);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();
	
	// Add a comment line with current values of variables :	
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
	int pitch;
	int pitchIndex;

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
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'z':
			mNote=Notes::Db;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 's':
			mNote=Notes::D;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'e':
			mNote=Notes::Eb;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'd':
			mNote=Notes::E;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'f':
			mNote=Notes::F;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 't':
			mNote=Notes::Gb;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'g':
			mNote=Notes::G;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'y':
			mNote=Notes::Ab;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'h':
			mNote=Notes::A;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'u':
			mNote=Notes::Bb;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		case 'j':
			mNote=Notes::B;
			signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			pitchIndex = static_cast<int>(mNote);
			pitch = pitchIndex+octaveIndex*12;
			signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		default:
			// compilation error: jump to default:
			mNote=Notes::A;
			// signalsNotes[static_cast<int>(mNote)].volume = 0.5;
			// pitchIndex = static_cast<int>(mNote);
			// pitch = pitchIndex+octaveIndex*12;
			// signalsNotes[static_cast<int>(mNote)].frequency = pitchToFrequency(pitch);
			break;
		}
	// pitchIndex = static_cast<int>(mNote);
	// pitch=pitchIndex+octaveIndex*12;
	// targetFrequency=pitchToFrequency(pitch); // initialization
	// singleNote.frequency = targetFrequency;
	// singleNote.volume = 0.0;
	std::cout << "key pressed " << key << std::endl;

}

//--------------------------------------------------------------
void ofApp::keyReleased  (int key){
	// signals.clear();
	singleNote.volume = 0.;
	switch (key)
		{
		case 'q':
			mNote=Notes::C;
				signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'z':
			mNote=Notes::Db;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 's':
			mNote=Notes::D;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'e':
			mNote=Notes::Eb;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'd':
			mNote=Notes::E;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'f':
			mNote=Notes::F;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 't':
			mNote=Notes::Gb;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'g':
			mNote=Notes::G;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'y':
			mNote=Notes::Ab;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'h':
			mNote=Notes::A;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'u':
			mNote=Notes::Bb;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		case 'j':
			mNote=Notes::B;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		default:
			// compilation error: jump to default:
			mNote=Notes::A;
			signalsNotes[static_cast<int>(mNote)].volume = 0.0;
			break;
		}
}

// remove the frequency change with moving mouse
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
	int width = ofGetWidth();
	float widthPct = ((float)x)/ ((float)width); 
	float height = (float)ofGetHeight();
	float heightPct = ((height-y) / height);
	lowFrequency = 20000 * heightPct;
	lowQ = 0.01 + 0.99 * widthPct;
	lowFilter = lowPassFilter(lowFrequency, lowQ);
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
	// lAudioPreviousValues.x_1 = lAudio[bufferSize - 1];
	// lAudioPreviousValues.x_2 = lAudio[bufferSize - 2];
	// lAudioPreviousValues.y_1 = lAudioFiltered[bufferSize - 1];
	// lAudioPreviousValues.y_2 = lAudioFiltered[bufferSize - 2];
	// rAudioPreviousValues.x_1 = rAudio[bufferSize - 1];
	// rAudioPreviousValues.x_2 = rAudio[bufferSize - 2];
	// rAudioPreviousValues.y_1 = rAudioFiltered[bufferSize - 1];
	// rAudioPreviousValues.y_2 = rAudioFiltered[bufferSize - 2];
	initSignal();
	for(auto & signal : signals ){
		addSignal(signal);
	}
	for(auto & signal: signalsNotes){
		addSignal(signal);
	}
	applyFilter(lowFilter);
	for (size_t i = 0; i < buffer.getNumFrames(); i++){
		buffer[i*buffer.getNumChannels()    ] = lAudioFiltered[i]; // = sample * volume * leftScale;
		buffer[i*buffer.getNumChannels() + 1] = rAudioFiltered[i]; // = sample * volume * rightScale;
	}
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
