/*
* Piano synthesis based on "Physics-Based Sound Synthesis of the Piano" (Balázs Bank)
* see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* The below code tries to use settings that approximately match the physics
* of a real piano. (Even if it may not make any difference on the output
* sound quality.)
*
* todo: implement dampers
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/

#include <stdlib.h>
#include <math.h>
//#include <stdio.h>

#include <emscripten.h>

#include "NoteUtil.h"
#include "PianoVoice.h"

// params that can be manipulated from the outside
static float sVolume=				5.0;
static float sTweakBridgeZ=			1.0;
static float sHammerZ=				0.0;	// impedance of hammer
static float sTweakHammerV=			1.0;
static float sTweakStringLen=		1.0;
static float sTweakStringRadius=	1.0;
static float sTweakStringB=			1.0;


void debugAll();

#define MAX_STRINGS 3

PianoVoice::PianoVoice(float sampleRate) {	
	_soundboardFilter = new SoundboardFilter(sampleRate);

	_hammer = new Hammer(sampleRate);

	for(int k=0; k<MAX_STRINGS; k++) {
		_string[k] = new PianoString(sampleRate);
	}
	strikeNote(0, 0); // init 
	
//	debugAll();
}

PianoVoice::~PianoVoice() {
	for(int k=0; k < _n; k++) {
		delete _string[k];
	}  
	delete _hammer;
	delete _soundboardFilter;
}

// #define USE_FIXED_TENSION


float getStringLength(int midiNote, float freq) {
#ifdef USE_FIXED_TENSION
	return 0; // NO-OP
#else
	/*
	realworld strings are typically longest (112-128cm) for deepest bass
	note and shortest (5-11cm) for highest treble note. generally speaking, string 
	length follows some kind of sigmoid distribution (that flattens off towards the
	treble end) and there is a discontinuity at piano key 27, i,e. at the switch 
	from the shorter bass-bridge strings to the (at first) longer treble bridge 
	strings. see image p.980 http://www.cs.ioc.ee/~stulov/appl08.pdf
	
	below formulas are NOT based on any real piano but just guesstimates	
	*/
	midiNote= 	sanitizeNote(midiNote);
	float x= 	getLinearPos(midiNote);

	// arbitrary sigmoid formula to create 1 to 0 curve that flattens off at the end
	// (could be improved: currently the length actually gets longer towards the end)
	// http://www.fooplot.com/#W3sidHlwZSI6MCwiZXEiOiIoMS0oMC40OCp4KzIuMTUqTWF0aC5wb3coeCwyKS0xLjYzMypNYXRoLnBvdyh4LDMpKSkqMS4xNzcrMC4wNjMiLCJjb2xvciI6IiMwMDAwMDAifSx7InR5cGUiOjEwMDAsIndpbmRvdyI6WyIwIiwiMSIsIjAiLCIxLjUiXX1d
	
	float l= 1 - (0.48*x + 2.15*pow(x,2) - 1.633*pow(x,3)); 
	
	if (l < 0) l= 0;	// just in case
	
	if (midiNote >= (27 + MIDI_OFFSET)) {
		l*= 1.1772727;				// restart with longer strings for treble range
		l+= 0.063;
	}
	
	// actually the resulting length (or wire radii) seem to be completely 
	// off - resulting in tensions that are far from those used on real 
	// pianos.. (see USE_FIXED_TENSION getTension() impl)

	return l * 1.30 * sTweakStringLen;	// maximum of longest bass string in meters
#endif
}

#ifdef USE_FIXED_TENSION
float getIdealStringLength(float t, float freq, float lmd) {
	return sqrt( t / (4*pow(freq, 2)*lmd) );
}
float getTension(float l, float lmd, float freq, int midiNote) {
	// lets try to do this the other way round.. chose
	// the string length in the end .. based on the desired tension..
	// see getIdealStringLength().
	
	// example Steinway D: distribution of tensions starting from deepest 
	// bass key: [#1: 1668 -#20:], [#21-33: >889], [#34: -  #53: 756] 
	// [#54: 733 - #71: 800], [#72: 822 - #88: 667]
	
	int note= midiNote- MIDI_OFFSET;
	if (note < 20) {
		return (1.0 - (float)note/19.0)*(1668-889) + 889;
	} else if (note < 33) {
		return 889;
	} else if (note < 53) {
		return (1.0 - (float)(note-33)/19.0)*(889-756) + 756;
	} else if (note < 71) {
		return ((float)(note-53)/17.0)*(800-733) + 733;
	} else {
		return (1.0 - (float)(note-71)/16.0)*(822-667) + 667;
	}
}

#else
	
float getTension(float l, float lmd, float freq, int midiNote) {
	// "t influences the spectrum of its vibrations it is disirable that this
	// spectrum is similar for adjacent strings, i.e. piano design starts with
	// the target-t and then selects a suitable wire-density"

	// string tension t=(2*freq)*(2*freq)*l*M  => where M is the mass of the
	// entire string: M= l*lmd (see http://www.cs.ioc.ee/~stulov/appl08.pdf)
	
	// note: for wrapped strings the tension is calculated using lmd that includes
	// the "wrapping". (see Stulov's "Physical modelling of the piano string scale")
	
	return pow(2*freq, 2) * l * l*lmd;		// newtons
}
#endif


float getStringRadius(int midiNote, float freq) {	// result in m
	/*
	piano stings are as short as possible, as tense as possible and as heavy as 
	possible (see diameter of string): this reduces displacement amplitudes thereby
	reducing losses due to drag & heat dissipation. string thickness increases
	"stiffness" which leads to undesirable inharmonics. stiffness of thick bass 
	strings is therefore reduced by using wrapping wire that increases weight without 
	increasing stiffness

	string tensions should be more or less equal to avoid stress on the frame 
	on real hardware (which may be difficult to achieve due to limited choice of
	available wire thickness): step-width of wire-diameter is usually 0.025mm for  
	steel and 0.05mm for winding wire.
	
	diameters of regular wires may vary from around 1.7 - 0.4mm.. and wound bass 
	strings may have diameters of 6mm (and more) with a core of 1.4mm (which is 
	relevant for the stiffness of the string).. (e.g. on a Steinway Model O regular 
	wires are used starting from 442Hz, i.e. everything below key 49 uses wound wires)
	*/
	midiNote=	sanitizeNote(midiNote);	
	float x= 	getLinearPos(midiNote);

	float r= (1.6 - 1.2*x)/2000; 	// 1.6-0.4mm
	
	// limit available radii just for fun..
	r= round(r*80000) / 80000;	// regular steel wire limitation

	return r * sTweakStringRadius;
}

float getMassDensity(float r) {	// linear density (aka "mass per unit length")
	// the average density of piano string wire depends on the supplier/string type
	// and may vary between 7.81 and 7.85 g/cm3 (the number given by Stulov is 7.86)

	// ISSUE: wrapped wires use different materials and the respective steel based 
	// density calculation is all wrong in that scenario (see getWrappedMassDensity())

	// based on their "total radius" wrapped strings have a significantly lower 
	// combined density: for thin (0.2mm) wrappings it might be 6900 but for thick 
	// (3.7mm) wrappings it might be 4600.. => if the linear mass density was to be 
	// calculated using r and the below formula..
	
	float density= 7850.0;				// kg/m3
	return M_PI*pow(r, 2) * density;
}

float getWrappedMassDensity(int midiNote, float lmd) {
	// add the effect of the wrapping wires to the string's core wire
	
	// bass strings: "The linear mass density of these strings must rise smoothly 
	// from a value of u = 7.81 g/m (the linear mass density of the string n = 30 
	// [which isn't the last of the wound wires!]) up to u = 290 g/m (the linear 
	// mass density of the first string)"

	midiNote= sanitizeNote(midiNote);	
	int note= midiNote-	MIDI_OFFSET;
	
	float mul= 1;
	if (note < 30) {		
		// without knowledge of the actual wrapping used, this is pure conjecture..
		
		mul= 1.0+ (1.0-((float)note/29))*18;	// hand-tuned to reach ~290 g/m 
	}
	return lmd * mul;
}

float getImpedance(float t, float lmd) {
	// impedance of string: Z= "sqrt(t*u)" u=linear density of the string see p.48 
	// "Principles of Vibration and Sound" by Rossing, et al
			
	// issue: formula is for "string of infinite length"
	return sqrt(t*lmd);		// typically around 3-4 for real strings
}

float getInharmonicity(float r, float l, float t) {
	// "Inharmonicity B largely affects the lowest and highest notes in the piano"	
	// (the actual core of a wrapped string determines the inharmonicity, i.e. in 
	// this context r should not include the wrapping but only the core wire)

	// Euler Bernoulli model (see "Linear stiff string vibrations in musical acoustics: 
	// assessment and comparison of models" (#A)) is used here to approximate the effect.	
	// note: the formula is incorrect for wrapped strings (see Fletcher and Rossing 1998)
	
	// Young's modulus (measure of stiffness/linear elasticity).. see 
	// https://www.stephenpaulello.com/sites/default/files/paulello/intros-de-pages/stephenpaulellomusicwire.pdf
	const  float E = 202e9;	// N/m2
	
	// meaning of vars used in below comments
	// S= PI*r*r		=> the cross section
	// K= r/2			=> radius of gyration for wire
	// k= E*S*K*K		=> models "bending stiffness"

	// 1) in "Piano strings with reduced inharmonicity" 2.2) the formula is given as: 
	// b= PI^2*k/t*l^2 (which agrees with the respective formula in (paper #A))

	return pow(M_PI, 3) * pow(r*sTweakStringB, 4) * E / (4.0*pow(l, 2)*t);
		
	// 2) see "Principles of vibration and sound" (#b) p.61 (2.67): the text does NOT state 
	// what B is, but supposing it is the 1st differentiation of the 4th term (n^2*PI^2*b^2/8, 
	// and b=r^2/l*sqrt(PI*E/t)) then that leads to the same result as above:
	
//	return 2.0*pow(M_PI,3)*pow(r,4)*E / (8.0*pow(l,2)*t);
}

void debugAll() {
	for (int i= 0; i<88; i++) {
		int midiNote= MIDI_OFFSET+i;
		float freq= getFrequency(midiNote);
		float l= getStringLength(midiNote, freq);		// in m

		float r= getStringRadius(midiNote, freq); 		// in m
		float lmd= getMassDensity(r);
		float t0= getTension(l, lmd, freq, midiNote);	// in newtons
		lmd= getWrappedMassDensity(midiNote, lmd);

#ifdef USE_FIXED_TENSION
		l= getIdealStringLength(t0, freq, lmd);
#endif	
		printf("%d l: %f r: %f dens: %f\n", i+1, l, r, lmd);
	}
}

float imperfection(int i, int totalStrings) {
	
	// no point using multiple strings if they were identical..
	const float LEVEL[3] = { 1, 1.0005, 0.9995 };	
	
	if (totalStrings == 1) {
		return LEVEL[i];
	} else if (totalStrings == 2) {
		return LEVEL[i+1];
	} else {
		return LEVEL[i];
	}
}

int getNumberOfStringsUsed(int midiNote) {
	// this should be about right..
	if(midiNote < 31) {
		return 1;
	} else if(midiNote < 41) {
		return 2;
	} else { 
		return 3;
	}
}

//#define TEST_EXTREMES

#ifdef TEST_EXTREMES
int toggle= 1;
#endif

void PianoVoice::strikeNote(int midiNote, float velocity) {
#ifdef TEST_EXTREMES
	toggle = !toggle;
	if (velocity) midiNote= toggle? MIDI_OFFSET : MIDI_OFFSET + 87;
#endif
		
	float freq=	getFrequency(midiNote);					// note's fundamental frequency
	_n=			getNumberOfStringsUsed(midiNote);

	float l=	getStringLength(midiNote, freq);		// m
	float r=	getStringRadius(midiNote, freq); 		// m

	float lmd=	getMassDensity(r);	
	float t0=	getTension(l, lmd, freq, midiNote);		// newtons
	lmd= 		getWrappedMassDensity(midiNote, lmd);

	
#ifdef USE_FIXED_TENSION
	// ..or calculate it based on some target tension
	l= 			getIdealStringLength(t0, freq, lmd);
#endif
		
	float t= getTension(l, lmd, freq, midiNote);		// newtons	
	float zs= getImpedance(t, lmd);						// impedance of string
	
	_hammer->strike(midiNote, freq, r, zs, velocity * sTweakHammerV);

	_soundboardFilter->init(freq, midiNote);
	
	for(int i=0; i< _n; i++) {
		float tweak= imperfection(i, _n);
		float freqTw= tweak*freq;
			
		t= getTension(l, lmd, freqTw, midiNote);
		zs= getImpedance(t, lmd);
		
		//"If e.g. the impedance of the termination decreases with frequency,
		// the decay times of the higher harmonics will be smaller than those 
		// of the lower ones. ..., the average impedance of real piano bridges 
		// is nearly constant"
		float zb= (5000 - 500*getLinearPos(midiNote)) * sTweakBridgeZ;
	
		float b= getInharmonicity(r, l, t0);	// considering only the string's core wire radius

		_string[i]->init(freqTw, midiNote, b, zs, zb, sHammerZ, tweak);
	}	
}

float PianoVoice::getStringVelocity() {
	// determine "displacement of the string" as a feedback signal to the hammer..
	float v= 0.0;
	for(int i= 0; i<_n; i++) {
		v+= _string[i]->getStringVelocity();
	}
	return v/_n;
}

long PianoVoice::renderOutput(float *buffer, int len) {

	for(int i= 0; i<len; i++) {
				
		float v= getStringVelocity();		
		float fin= _hammer->getForceInput(v);
		
		float sample = 0.0;
		for(int j=0; j<_n; j++) {
			sample+= _string[j]->getBridgeSignal(fin);
		}
		sample/= _n;
				
		sample= _soundboardFilter->process(sample);

		buffer[i] = sample * sVolume;
	}
	return len;
}

void PianoVoice::setStringTweaks(float len, float thickness, float b, float sumZ, float detune, bool propagate) {	
	sTweakStringLen= len;
	sTweakStringRadius= thickness;
	sTweakStringB= b;
		
	for(int i= 0; i<_n; i++) {
		_string[i]->setStringTweaks(sumZ, detune, propagate);
	}
}

void PianoVoice::setHammerTweaks(float zh, float vh) {
	sHammerZ= zh;
	sTweakHammerV= vh;
}

void PianoVoice::setBridgeTweaks(float volume, float g, float decay, float freq, float q, float bridgeZ) {
	_soundboardFilter->setLossFilterSettings(g, decay);
	_soundboardFilter->setLowpassSettings(freq, q);

	sVolume= volume;
	sTweakBridgeZ= bridgeZ;
}