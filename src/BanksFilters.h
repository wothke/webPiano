/*
* Special purpose filter configurations specified in "Physics-Based Sound Synthesis 
* of the Piano" (Balázs Bank) see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#ifndef BANKSFILTERS_H
#define BANKSFILTERS_H

#include "Filter.h"
#include "BiquadFilter.h"


/*
* Tunable dispersion filter based on a cascade of 2nd order Thiran allpass filters.
*
* see "Tunable Dispersion Filter Design for Piano Synthesis" (by Jukka Rauhala and Vesa Välimäki)
* https://aaltodoc.aalto.fi/bitstream/handle/123456789/2974/article1.pdf?sequence=2&isAllowed=y
*/
class DispersionFilter : public IFilter {
public:
	DispersionFilter();
	virtual ~DispersionFilter();

	virtual float process(float input);
	
	virtual float getPhaseDelay(float freq, float sampleRate);
	virtual float getGroupDelay(float freq, float sampleRate);
	
	void init(float freq, float b);
private:
	float getDelayParameter(float b, float freq, bool useLowFreqMode);

private:
	int _num;					// number of cascaded filters
	ThiranFilter _allPass[4];	// block of 2nd-order allpass filters (1 or cascade of 4 is used)
};


/**
* Postprocessing filter that approximates "soundboard" behavior.
*
* The main feature is use of a "feedback delay network" as suggested 
* in http://home.mit.bme.hu/~bank/thesis/pianomod.pdf , i.e. for losses, one-pole 
* filters are used in series to the delay lines.
*
* Also see "Circulant and Elliptic Feedback Delay Networks for Artificial Reverberation" 
* (by Rocchesso & Smith).
*/
class SoundboardFilter {
public:
	SoundboardFilter(float sampleRate);	
	void init(float freq, int midiNote);
	
	float process(float input);
	
	// change lowpass filter settings
	void setLowpassSettings(float freq, float q);
	// change loss filter settings
	void setLossFilterSettings(float g, float decay);

private:
	static void initStaticParams();
private:
	float _sampleRate;
	// lowpass filter settings
	float _lowpassFreq;
	float _lowpassQ;
	// loss filter settings
	float _lossG;
	float _lossDecay;

	// circular feedback matrix (scattering matrix) contains all feedback gains
	static float _A[8][8];	
	static float _c[8];	// output coefficient vector

	float _in[8];	// local var put here as an optimization
	float _out[8];

	// loss filters designed in a way suggested in [Jot and Chaigne 1991]
	DelayFilter _delayFilter[8];
	LossFilter _lossFilter[8];
	
	BiquadFilter _shapeFilter;
};
#endif