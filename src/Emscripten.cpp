/*
* Piano synthesis based on "Physics-Based Sound Synthesis of the Piano" (Balázs Bank)
* see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* Emscripten based interface to the JavaScript world. (Quick'n'dirty interface
* to support my 3-voice Pachelbel.)
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <emscripten.h>

#include "BanksFilters.h"

#include "PianoVoice.h"


#define MAX_SCOPE_BUFFERS 1
#define SAMPLE_RATE 44100
// SAMPLE_RATE/50 
#define BUF_SIZE 882

// output "scope" streams corresponding to final audio buffer
static int16_t*	_scope_buffers[MAX_SCOPE_BUFFERS];	
static int16_t 	_scope_buffer[BUF_SIZE];	

float 			_soundBuffer[BUF_SIZE];

float 			_soundBufferV1[BUF_SIZE];
float 			_soundBufferV2[BUF_SIZE];
float 			_soundBufferV3[BUF_SIZE];

static uint32_t _number_of_samples_rendered = 0;

static PianoVoice *piano[3];	// hardcoded 3 "voices"

extern "C" void strikeNote(int voice, int note, float v0)  __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE strikeNote(int voice, int note, float v0) {
	if (piano[0] != 0) {
		piano[voice]->strikeNote(note, v0);
	}
}

extern "C" uint32_t initPiano()  __attribute__((noinline));
extern "C" uint32_t EMSCRIPTEN_KEEPALIVE initPiano() {

	_scope_buffers[0]= _scope_buffer;
	
	if (piano[0] == 0) {
		piano[0] = new PianoVoice( (float)SAMPLE_RATE);
		piano[1] = new PianoVoice( (float)SAMPLE_RATE);
		piano[2] = new PianoVoice( (float)SAMPLE_RATE);
	}
	return 0;
}						

extern "C" uint32_t getSampleRate() __attribute__((noinline));
extern "C" uint32_t EMSCRIPTEN_KEEPALIVE getSampleRate() {
	return SAMPLE_RATE;
}

extern "C" uint32_t getSoundBufferLen() __attribute__((noinline));
extern "C" uint32_t EMSCRIPTEN_KEEPALIVE getSoundBufferLen() {
	return _number_of_samples_rendered;	// in samples
}

extern "C" char* getSoundBuffer() __attribute__((noinline));
extern "C" char* EMSCRIPTEN_KEEPALIVE getSoundBuffer() {
	return (char*) _soundBuffer;
}

extern "C" int getNumberTraceStreams() __attribute__((noinline));
extern "C" int EMSCRIPTEN_KEEPALIVE getNumberTraceStreams() {
	return 1;	// just copy regular output
}

extern "C" const char** getTraceStreams() __attribute__((noinline));
extern "C" const char** EMSCRIPTEN_KEEPALIVE getTraceStreams() {
	return (const char**)_scope_buffers;	// ugly cast to make emscripten happy
}

extern "C" int32_t computeAudioSamples()  __attribute__((noinline));
extern "C" int32_t EMSCRIPTEN_KEEPALIVE computeAudioSamples() {
	if (piano[0] == 0) {
		_number_of_samples_rendered= 0;
	} else {
		_number_of_samples_rendered=  piano[0]->renderOutput(_soundBufferV1, BUF_SIZE);
		_number_of_samples_rendered=  piano[1]->renderOutput(_soundBufferV2, BUF_SIZE);
		_number_of_samples_rendered=  piano[2]->renderOutput(_soundBufferV3, BUF_SIZE);
	}
	for (int i= 0; i<_number_of_samples_rendered; i++) {
		_soundBuffer[i]= _soundBufferV1[i] + _soundBufferV2[i] + _soundBufferV1[i];
		
		_scope_buffer[i]= (int16_t)(_soundBuffer[i]*0x7fff);	// just in case there will be more..
	}
	return _number_of_samples_rendered;
}

extern "C" void setBridgeTweaks(float g, float decay, float freq, float q, float bridgeZ) __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE setBridgeTweaks(float g, float decay, float freq, float q, float bridgeZ) {
	if (piano[0] != NULL) {
		piano[0]->setBridgeTweaks( g, decay, freq,  q, bridgeZ);
		piano[1]->setBridgeTweaks( g, decay, freq,  q, bridgeZ);
		piano[2]->setBridgeTweaks( g, decay, freq,  q, bridgeZ);
	}
}

extern "C" void setHammerTweaks(float zh, float vh) __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE setHammerTweaks(float zh, float vh) {
	if (piano[0] != NULL) {
		piano[0]->setHammerTweaks( zh, vh);
		piano[1]->setHammerTweaks( zh, vh);
		piano[2]->setHammerTweaks( zh, vh);
	}
}

extern "C" void setStringTweaks(float len, float thickness, float b, float sumZ, float detune, int propagate) __attribute__((noinline));
extern "C" void EMSCRIPTEN_KEEPALIVE setStringTweaks(float len, float thickness, float b, float sumZ, float detune, int propagate) {
	if (piano[0] != NULL) {
		bool p= propagate != 0;
		piano[0]->setStringTweaks(len, thickness, b, sumZ, detune, p);
		piano[1]->setStringTweaks(len, thickness, b, sumZ, detune, p);
		piano[2]->setStringTweaks(len, thickness, b, sumZ, detune, p);
	}
}

