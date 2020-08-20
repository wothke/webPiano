/*
* Generic filters used in "Physics-Based Sound Synthesis of the Piano" (Balázs Bank)
* see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/

//#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Filter.h"

void saveDelete(float **buf) {
	if (*buf) {
		delete(*buf);
		(*buf)= 0;
	}
}
void assertBuffer(float **buf, int sOld, int sNew, bool doClear) {
	if (sOld < sNew) {
		if (sOld) {
			saveDelete(buf);
		}
		(*buf)= new float[sNew];		
	}
	if(doClear) {
		memset((*buf), 0, sNew*sizeof(float));	// note: "new" may return uninitialized memory
	}
}


// ------------------------ delay filter ----------------------------------

DelayFilter::DelayFilter() {}
DelayFilter::~DelayFilter() {
	saveDelete(&_circularBuffer);
}

void DelayFilter::init(int delay) {
	_delay= delay < 0 ? 0 : delay;
	if (_delay) {
		int sOld= _size;
		_size= next_pow2(_delay+1);
		assertBuffer(&(_circularBuffer), sOld, _size, true);

		_writeIdx= 0;
		_readIdx= (_size - _delay);	// just lag behind the writeIdx
		_overflowMask= _size - 1;
	}
}

float DelayFilter::readNext() {
	float result= _circularBuffer[_readIdx++];
	_readIdx&= _overflowMask;	
	return result;
}

void DelayFilter::writeNext(float input) {
  _circularBuffer[_writeIdx++]= input;
  _writeIdx&= _overflowMask;
}

float DelayFilter::process(float input) {
	if (!_delay) {
		return input;
	} else {
		writeNext(input);
		return readNext();
	}
}

float DelayFilter::getPhaseDelay(float freq, float sampleRate) {
	return 0;
}
float DelayFilter::getGroupDelay(float freq, float sampleRate) {
	return _delay/sampleRate;
}
int DelayFilter::next_pow2(int x) {
	return pow(2, ceil(log(x)/log(2)));
}

// ---------- generic IIR filter implementation -------------------
#define FFTN 1
IIRFilter::IIRFilter() {
	_n= _size= 0;
	
}

IIRFilter::~IIRFilter() {
	saveDelete(&_x);
	saveDelete(&_y);
	saveDelete(&_a);
	saveDelete(&_b);
	
}

void IIRFilter::orderInit(int n) {
	int s= n+1;
	
	assertBuffer(&_x, _size, s, true);
	assertBuffer(&_y, _size, s, true);
	
	assertBuffer(&_a, _size, s, false);
	assertBuffer(&_b, _size, s, false);
	
	if (_size < s) {
		_size= s;
	}	  
	_n= n;
}

int IIRFilter::getOrder() {
	return _n;
}

void IIRFilter::shiftBufferRight(float *buf) {
	for (int i= _n; i>0; i--) {
		buf[i]= buf[i-1];
	}		
}

float getAngularFreq(float freq, float sampleRate) {
	return (float)freq *2 * M_PI / sampleRate;
}

float IIRFilter::getGroupDelay(float freq, float sampleRate) {
	if (!_n) return 0;
	
	// see http://www.cjs-labs.com/sitebuildercontent/sitebuilderfiles/GroupDelay.pdf
	// "Group Delay is defined as the negative derivative (or slope) of the phase response vs. frequency." or

	// The group delay is defined as the derivative of the phase with respect to angular 
	// frequency and is a measure of the distortion in the signal introduced by phase 
	// differences for different frequencies.
	  
	float f1= freq - 1;
	float w1= getAngularFreq(f1, sampleRate);

	float f2= freq + 1;
	float w2= getAngularFreq(f2, sampleRate);

	return (w2*getPhaseDelay(f2, sampleRate) - w1*getPhaseDelay(f1, sampleRate)) / (w2-w1);
}

float IIRFilter::getPhaseDelay(float freq, float sampleRate) {
	if (!_n) return 0;
	
	float w= getAngularFreq(freq, sampleRate);

	float re= 0.0, im= 0.0;

	for(int i=0;i<=_n; i++) {
		re+= cos(i*w)*_b[i];
		im-= sin(i*w)*_b[i];
	}
	float phase= atan2(im, re);	
	re= im= 0.0;

	for(int i=0; i<=_n; i++) {
		re+= cos(i*w)*_a[i];
		im-= sin(i*w)*_a[i];
	}

	phase-= atan2(im, re);
	return fmod( -phase, 2 * M_PI ) / w;
}  

float IIRFilter::process(float input) {	
	// see https://ccrma.stanford.edu/~jos/filters/Difference_Equation_I.html

	if (!_n) return input;

	shiftBufferRight(_x);
	shiftBufferRight(_y);

	float output= _b[0] * input;

	for (int i= 1; i<(_n+1); i++) {
		output+= _b[i] * _x[i];
		output-= _a[i] * _y[i];
	}
	_x[0]= input;
	_y[0]= output;
	
	return output;
}

// ---------- loss filter implementation -------------------

LossFilter::LossFilter() : IIRFilter() {
	IIRFilter::orderInit(1);
}
LossFilter::~LossFilter() {
}

void LossFilter::init(float freq, float c1, float c3) {
	
	float g= 1.0 - c1/freq;			// see "c1= freq * (1 - g)"
	
	// solve quadratic equation (see "c3= -freq * (a1 / 2 * (a1+1)^2))"
	float z= (4.0*c3 + freq) / (4.0*c3);
	float a1= -z + sqrt(pow(z, 2.0) - 1.0);	
	
	_b[0]= g * (1.0 + a1);
	_b[1]= 0;
	_a[0]= 1;
	_a[1]= a1;
}

// ---------- thiran filter implementation -------------------

ThiranFilter::ThiranFilter() : IIRFilter() {}
ThiranFilter::~ThiranFilter() {}

void ThiranFilter::init(int n, float d) {
	// see http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.124.9212&rep=rep1&type=pdf
	// "Implementation of Control of a Real-Time Guitar Synthesizer (Hiipakka)" p.24

	if(d <= 1.0) {
		// just pass through unfiltered
		IIRFilter::orderInit(0);
	} else {	
		IIRFilter::orderInit(n);

		for(int i=0; i<=n; i++) {

			double ak= (i%2==1) ? -nChooseK(n, i) : nChooseK(n, i);

			for(int j=0; j<=n; j++) {
				ak*= ((double)d - (double)(n - j));
				ak/= ((double)d - (double)(n - i - j));
			}
			_a[i]= (float)ak;
			_b[n-i]= (float)ak;
		}
	}
}

double ThiranFilter::nChooseK(int n, int k) {
	//	"binomial coefficient" aka "nCr" aka "n choose k"
	
    if (k > n) return 0;
    if (k * 2 > n) k= n-k;		//optimization for large R: N choose R = N choose (N-R)
    if (k == 0) return 1;

    long result= n;
    for( int i= 2; i <= k; ++i ) {
        result*= (n-i+1);
        result/= i;
    }
    return (double)result;
}


