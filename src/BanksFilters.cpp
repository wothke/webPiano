/*
* Special purpose filter configurations specified in "Physics-Based Sound Synthesis 
* of the Piano" (Balázs Bank) see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/

#include <math.h>
#include <string.h>
#include <stdio.h>

#include "NoteUtil.h"
#include "BanksFilters.h"



// ------------ Tunable dispersion filter --------------------

// dispersion: ".. caused by the stiffness of the string. It stretches the
// partial frequencies higher compared to harmonic frequencies"

// dispersion=> velocity of transverse waves is dependent on frequency

DispersionFilter::DispersionFilter() {}
DispersionFilter::~DispersionFilter() {}

/*
* Computes the delay parameter from the inharmonicity/dispersion coefficient B and the 
* fundamental frequency.
*
* CAUTION: 27.5Hz is the lowest supported frequency! (whould need to be fixed for 
* grand pianos with extra keys..)
*/
float DispersionFilter::getDelayParameter(float b, float freq, bool useLowFreqMode) {

	float c1, c2, k1, k2, k3;

	if(useLowFreqMode) {	// for N=2 and _num=4
		k1= -0.00050469;
		k2= -0.0064264;
		k3= -2.8743;
		c1= 0.069618;
		c2= 2.0427;
	} else {				// for N=2 and _num=1
		k1= -0.0026580;
		k2= -0.014811;
		k3= -2.9018;
		c1= 0.071089;
		c2= 2.1074;
	}

	// "A good solution to parameterize coefficients and seems to be to use 
	// logarithmic scale and to approximate with a second-order polynomial" ..
	float logB= log(b);
	float cd= exp(c1*logB + c2);
	float kd= exp(k1*pow(logB, 2) + k2*logB + k3);
	
	float ikey= log(freq*NOTEUP/LOWEST_FREQ) / log(NOTEUP);
	
	return exp(cd - ikey*kd);
}

void DispersionFilter::init(float freq, float b) {
	const int limit= (329+349)/2;	// split between piano keys 44 and 45
		
	if(freq > limit) {	// match keys: 45–88
		_num= 1;	// "single second-order allpass filter" for all higher notes

		float d= getDelayParameter(b, freq, false);
		_allPass[0].init(2, d);

	} else {			// match keys: 1-44
		_num= 4;	// "cascade of four second-order allpass filters"

		float d= getDelayParameter(b, freq, true);
		for(int i=0; i<_num; i++) {
			_allPass[i].init(2, d);		
		}
	}
}

float DispersionFilter::process(float input) {
	for(int i=0; i<_num; i++) {
		input= _allPass[i].process(input);
	}
	return input;
}

float DispersionFilter::getPhaseDelay(float freq, float sampleRate) {
	return _allPass[0].getPhaseDelay(freq, sampleRate);
}

float DispersionFilter::getGroupDelay(float freq, float sampleRate) {
	return _allPass[0].getGroupDelay(freq, sampleRate) * _num;
}



// ------------------------ SoundboardFilter ----------------------------------

// "eight delay line feedback delay network with gradually increasing delay 
// "lengths.. These are relative primes, in order to avoid harmonic structure. "
// not primes: 87, 492, 687, 721 (but hopefully relative primes).. also
// 687 kind of sticks out when looking at the curve
//	static const int DELAYS[8]= { 37, 87, 181, 271, 359, 492, 687, 721 };	// see Bank's thesis p.67

// why not just use all primes and also use a more uniform gradient:
static const int DELAYS[8]= { 37, 89, 181, 271, 359, 491, 613, 719 };

// since "input coefficient vector" b contains always 1
// in this context I completely ditched it here
float SoundboardFilter::_A[8][8];
float SoundboardFilter::_c[8];


SoundboardFilter::SoundboardFilter(float sampleRate) {	
	_sampleRate= sampleRate;

	initStaticParams();
	
	for(int i=0; i<8; i++) {
		// no not reset on "init" to avoid clicks
		_delayFilter[i].init(DELAYS[i]);
	}

	setLowpassSettings(10000, 0.707);
	setLossFilterSettings(0.95, 1.2);
}


void SoundboardFilter::setLossFilterSettings(float g, float decay) {
	_lossG= g;
	_lossDecay= decay;
}

void SoundboardFilter::setLowpassSettings(float freq, float q) {
	_lowpassFreq= freq;
	_lowpassQ= q;
	
	// FIXME todo: add more sophisticated shaping filter 
	// (for now just get rid of the high pitched clangor
	// introduced by the below filters)
	_shapeFilter.init(LPF, _sampleRate, _lowpassFreq, _lowpassQ, -999);	
}

void SoundboardFilter::init(float freq, int midiNote) {
	if (midiNote>74)  {
		// hack fixme: for some reason with higher frequencies
		// something breaks down miserably (also see p and K 
		// used in Hammer impl..) => investigate
		freq= 587.329529;
	}

	for(int i=0; i<8; i++) {		
		float f0=freq/DELAYS[i];

		float c1= f0*(1.0-_lossG);
		float v2= pow(2.0*M_PI*f0, 2.0);
		float c3= (1.0 - _lossDecay*c1)/(_lossDecay*v2);

		_lossFilter[i].init(f0, c1, c3);
	}
//	memset(_out, 0, sizeof(float)*8 );
}

void SoundboardFilter::initStaticParams() {

	// see chapter 5.5.4 "The soundboard"
	// "all the output signals of the delay lines after the loss filter are summed, 
	// multiplied by -1/4 and fed into the input of all the delay line".. ".. Better 
	// solutions can be obtained by shifting the feedback matrix to the right with one step"
	
	const float e= -0.25;
	// 1st row of the circular feedback matrix
	const float r0[8]= {e, static_cast<float>(1.0+e), e, e, e, e, e, e};
	
	for(int i=0; i<8; i++) {
		for(int j=0; j<8; j++) {
			_A[i][j]= r0[(8 + (j-i)) % 8];
		}
		
		//"..canceling of zeros and poles by using 1) and 2) 
		// having entries equal to 1, entries equal to -1, and 
		// zeros for the remaining entries.."
		
		
		// ".. the input and output coefficient vectors b and c were 
		// chosen as suggested in [Rocchesso and Smith 1997".... =>
		// paper does not contain any clue with regard to the actually 
		// used c - but the below seems to get the job done :-( 
		
		_c[i]= (i%2 == 1) ? e : -e;
	}
}

float SoundboardFilter::process(float input) {
	float in[8];
	
	for(int i= 0; i<8; i++) {
		in[i]= input;
		
		for(int j=0; j<8; j++) {
			in[i]+= _A[i][j] * _out[j];
		}
	}

	float output= 0;  
	for(int i= 0; i<8; i++) {
		// "For the losses, one-pole filters were used in 
		// series to the delay lines"
		
		_out[i]= _lossFilter[i].process( _delayFilter[i].process(in[i]) );
		output+= _c[i] * _out[i];
	}
	
	// Bank: "different filters should be used depending on the string.."
	
	// "For real-time applications, ... second or third order IIR shaping filters 
	// are applied for all the different regions of the soundboard."
	
	output= _shapeFilter.process(output);
	
	return output;
}

