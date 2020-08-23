/*
* Piano hammer model from "Physics-Based Sound Synthesis of the Piano" (Balázs Bank)
* see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#include <math.h>
#include <stdio.h>

#include "NoteUtil.h"
#include "Hammer.h"

#define min(x,y) ( \
    { __auto_type __x = (x); __auto_type __y = (y); \
      __x < __y ? __x : __y; })


float AbstractHammer::getHammerMass(float linPos) {
	// https://www.speech.kth.se/music/5_lectures/conklin/thehammers.html#:~:text=The%20largest%20bass%20hammers%20may,this%20would%20increase%20manufacturing%20problems.
	// i.e. in modern pianos the hammer mass linearly decreases from
	// around 12g to 3.5g with the ~first 12 bass keys plateauing at ~11g
	
	return (linPos<(12.0/88)) ? 0.011 : (12.0-linPos*(12.0-3.5))/1000;
}

float AbstractHammer::getStiffnessExponent(float linPos) {
	// hammer's stiffness exponent

	// http://homes.ioc.ee/stulov/jasa.pdf :
	//"..Hall and Askenfelt  measured the values of p for 16 real samples of
	// hammers. For the load force F ranging from 0.55 to 35 N these hammers
	// had p between 1.5 and 3.5 with no definite trend of p from bass to
	// treble. One hammer was exceptional, with p =5.0. ... In the author's
	// opinion, the most suitable values of p for a good grand piano are 2<p<3,
	// while p>>3 gives too much contrast in tone when playing fortissimo
	// versus pianissimo"
	
	return 2.2 + 0.7 * linPos;
}

float AbstractHammer::getStiffnessCoefficent(float linPos, float p) {
	// hammer's stiffness coefficent

	// see "Pysical modeling of the piano: An investigation into the effect of
	// string stiffness of the hammer string interaction":
	// K for C2/16 (4e08), C4/40 (4.5e09), C7/76 (1e12)

	// for some reason impl fails miserably when using multiple orders of difference
	// like the above data suggests.. the below is therefore just tuned by hand..
	// it sounds about alright 
	return 3E8 + 3E8 * linPos*4.0;	// use 5x higher K for highest notes
}


class Integrator {
public:
	Integrator() {}
	~Integrator() {}
	
    void init(float sampleRate, float out= 0.0);
    float process(float in);
private:	
    float _dT;
    float _out;
};

void Integrator::init(float sampleRate, float out) {
	_dT= 1.0 / sampleRate;	// avoid "expensive" division later
	_out= out;
}

float Integrator::process(float in){
	_out = _out + in*_dT;
	return _out;
}

// ----------------------- Bank's multi-rate hammer impl ------------------------------------


// hammer model runs at double sampling rate to avoid instability problems
Hammer::Hammer(float sampleRate) : AbstractHammer(sampleRate * 2) {
	
	_dVint= new Integrator();
	_dAint= new Integrator();
}

Hammer::~Hammer() {
	delete _dVint;
	delete _dAint;
}

void Hammer::strike(int note, float freq, float r, float z, float v0) {
	// issue/todo: smaller hammers would have higher velocities, i.e. v0
	// should NOT be constant.. (some tuning would not hurt)
	
	_ah= _lastVin= _fout= _vh= 0.0;
	_i2Z= 1.0 / (2.0*z);			// avoid "expensive" division later

	float linPos= getLinearPos(note);	
	
	_iM = 1.0 /	getHammerMass(linPos);	
	_p= getStiffnessExponent(linPos);
	_k= getStiffnessCoefficent(linPos, _p); 

	// fixme: something in the current configuration is causing "clapping"
	// sounds and sporadic "rumbling" (see "autoplay" )
	
	_dVint->init(_sampleRate);
	_dAint->init(_sampleRate, v0*2);
	
	_delayFilter.init(1);
}

float Hammer::runHammerModel(float vin) {
	//  discretization of a differential equation: displacement of the
	// string can be determined by integrating the velocity signal
	
    float dV= _vh - vin - _fout*_i2Z;
    float dY= _dVint->process(dV);	// felt compression

	// keep track of string displacement in between
	// the time steps of the digital waveguide
	
    _ah= -_fout*_iM;				// hammer accelleration
    _vh= _dAint->process(_ah);		// hammmer velocity
	
	// power-law for felt (see Eq. 2.1) 
	
	// CAUTION: an "excessive" stiffness coefficent may lead to 
	// instable behavior, i.e. infinite force output .. which will 
	// wreak havoc in the later output filters..
	
	float f= (dY>0.0) ? min(1000.0, _k*pow(dY, _p)) : 0.0;	// hack: use min to avoid crashes
	
    return _delayFilter.process(f);
}

float Hammer::getForceInput(float vin){
	
	// "double" the sample frequency by creating
	// an additional interpolated vin signal
	
    float vin2= (vin + _lastVin) * 0.5;
	_lastVin= vin;
	
	_fout= runHammerModel(vin2);
	float out= _fout;
    _fout= runHammerModel(vin);
	    		
    return (out + _fout) * 0.5; 	// interaction force/force input to the string (N)
}


