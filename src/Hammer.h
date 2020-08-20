/*
* Piano hammer model from "Physics-Based Sound Synthesis of the Piano" (Balázs Bank)
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#ifndef HAMMER_H
#define HAMMER_H

#include <stdio.h>

#include "Filter.h"

class Integrator;

/*
* Generic hammer interface.
*/
class AbstractHammer {
public:
	AbstractHammer(float sampleRate) { _sampleRate= sampleRate; }
	virtual ~AbstractHammer() {}

	/*
	* @param note midi-code of the played note
	* @param freq fequency of the played note (redundant to note param)
	* @param r radius of the string
	* @param z impedance of the string
	* @param v0 initial hammer velocity
	*/
	virtual void strike(int note, float freq, float r, float z, float v0) = 0;
	
	/*
	* @param vin		incoming string velocity
	* @return fin		force input to the string
	*/
	virtual float getForceInput(float vin) = 0;

protected:	
	float getHammerMass(float linPos);
	float getStiffnessExponent(float linPos);
	float getStiffnessCoefficent(float linPos, float p);

protected:
	float _sampleRate;
};

/*
* Balázs Bank's  multi-rate hammer model.
*
* see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf p.54
*/
class Hammer : public AbstractHammer {
public:
	Hammer(float sampleRate);
	~Hammer();

	void strike(int note, float freq, float r, float z, float v0);

	float getForceInput(float vin);
	
private:
	float runHammerModel(float vin);
private:
	float _iM;
	float _i2Z;
	float _k;
	float _p;
	
	float _lastVin;	// previous velocity input
	float _vh;		// velocity
	float _ah;		// acceleration hammer
	float _fout;	// force signal computed by the power law in the previous time instant
	
	Integrator *_dVint;
	Integrator *_dAint;
	DelayFilter _delayFilter;
};

#endif
