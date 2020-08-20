/*
* "Piano string" implementation based on a digigal waveguide
* with added filters for secondary effects.
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/

#ifndef PIANOSTRING_H
#define PIANOSTRING_H

#include <vector>
 
#include "BanksFilters.h"

class Node;
class Waveguide;

/*
* "The string is modeled by a digital waveguide, where the 
* losses and the dispersion of the string are consolidated 
* to one point [Smith 1987, 1992]. Its task is to control
* the frequencies of the partials and the decay times. 
* It also influences the spectra by a comb filtering effect."
*/
class PianoString {
public:
	PianoString(float sampleRate); 
	~PianoString();
	
	/*
	* @param b inharmonicity
	* @param z impedance string
	* @param zb impedance bridge
	* @param zh impedance hammer
	*/
	void init(float freq, int midiNote, float b, float z, float zb, float zh, float tweak); 

	// string velocity at the hammer's "striking point"
	float getStringVelocity();
	
	/*
	* @param fin force input from the hammer
	*/
	float getBridgeSignal(float fin);

	
	// tweak the default configuration
	void setStringTweaks(float sumZ, float detune, bool propagate);
	
private:
	float initStringSegments(float freq, int midiNote, float z, float t);
	void initLossDispersion(float freq, float b);
protected:
	DispersionFilter	_dispersionFilter; 
	LossFilter			_lossFilter;
	ThiranFilter		_fracDelayFilter;

	friend float tuningDelay(PianoString *s, float in);
	friend float leftReflect(PianoString *s, float in);
	friend float rightReflect(PianoString *s, float in);
private:
	float		_sampleRate;
	float		_i2Z, _2Z;		// impedance string
	float		_iZb;			// inverse impedance bridge
	
	Waveguide*	_stringS0; 		// string segment 1
	Waveguide*	_stringS1; 		// string segment 2
	Node*		_hammerNode;	// just to feed the waveguides
};


#endif
