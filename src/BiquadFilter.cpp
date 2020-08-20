/*
* Some standard filters (lowpass, highpass, etc) based on biquad-filter implementation.
*
* coefficients by Robert Bristow-Johnson
* see https://webaudio.github.io/Audio-EQ-Cookbook/Audio-EQ-Cookbook.txt
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/

#include <math.h>

#include "BiquadFilter.h"


BiquadFilter::BiquadFilter() {}
BiquadFilter::~BiquadFilter() {}

void BiquadFilter::init(BiquadType type, float sampleRate, float freq, float Q, float dBgain) {
	IIRFilter::orderInit(2);
	
    float w0=		2.0*M_PI*freq/sampleRate;
    float alpha=	sin(w0)/(2*Q);									// (case: Q)
	
	_init(type, w0, alpha, dBgain);	
}

void BiquadFilter::initBW(BiquadType type, float sampleRate, float freq, float BW, float dBgain) {
	IIRFilter::orderInit(2);
	
    float w0=		2.0*M_PI*freq/sampleRate;	
    float alpha=	sin(w0)*sinh( log(2.0)/2.0 * BW * w0/sin(w0) );  	// (case: BW)
	
	_init(type, w0, alpha, dBgain);
}

void BiquadFilter::init(BiquadTypeShelf type, float sampleRate, float freq, float S, float dBgain) {
	IIRFilter::orderInit(2);
	
	float A=	pow(10.0, (dBgain/40.0));
    float w0=	2.0*M_PI*freq/sampleRate;
	
    float co=	cos(w0);
    float si=	sin(w0);

    float alpha	= si/2 * sqrt( (A + 1.0/A)*(1.0/S - 1) + 2 );			// (case: S)
	
	switch (type) {
		case LOWSHELF: {
			float a0=  (A+1) + (A-1)*co + 2.0*sqrt(A)*alpha;
            _b[0]=  (    A*( (A+1) - (A-1)*co + 2.0*sqrt(A)*alpha ))/a0;
            _b[1]=  (2.0*A*( (A-1) - (A+1)*co                     ))/a0;
            _b[2]=  (    A*( (A+1) - (A-1)*co - 2.0*sqrt(A)*alpha ))/a0;
            _a[0]=  1.0;
            _a[1]=  ( -2.0*( (A-1) + (A+1)*co                     ))/a0;
            _a[2]=  (        (A+1) + (A-1)*co - 2*sqrt(A)*alpha)    /a0;
			}
			break;
			
		case HIGHSHELF: {
			float a0= (A+1) - (A-1)*co + 2.0*sqrt(A)*alpha;
            _b[0] = (     A*( (A+1) + (A-1)*co + 2.0*sqrt(A)*alpha ))/a0;
            _b[1] = (-2.0*A*( (A-1) + (A+1)*co                     ))/a0;
            _b[2] = (     A*( (A+1) + (A-1)*co - 2.0*sqrt(A)*alpha ))/a0;
            _a[0] =  1.0;
            _a[1] = (   2.0*( (A-1) - (A+1)*co                     ))/a0;
            _a[2] = (         (A+1) - (A-1)*co - 2.0*sqrt(A)*alpha)  /a0;
			}
			break;
	}
}

void BiquadFilter::_init(BiquadType type, float w0, float alpha, float dBgain) {
    float co=	cos(w0);
    float si=	sin(w0);

	switch (type) {
		case LPF: {
			float a0= 1.0 + alpha;
            _b[0]=	((1.0 - co)/2.0)/a0;
            _b[1]=	 (1.0 - co)     /a0;
            _b[2]=	((1 - co)/2.0)  /a0;
            _a[0]=	1.0;
            _a[1]=	 (-2.0*co)      /a0;
            _a[2]=	(1-0 - alpha)   /a0;
			}
			break;
			
		case HPF: {
			float a0= 1.0 + alpha;
            _b[0]=	( (1.0 + co)/2.0)/a0;
            _b[1]=	(-(1.0 + co))    /a0;
            _b[2]=	( (1.0 + co)/2.0)/a0;
            _a[0]=	1.0;
            _a[1]=	(-2.0*co)        /a0;
            _a[2]=	(1.0 - alpha)    /a0;
			}
			break;
			
		case BPFQ: {
			float a0= 1.0 + alpha;
            _b[0]=	(si/2.0)     /a0;  // =   Q*alpha
            _b[1]=	0;
            _b[2]=	(-si/2.0)    /a0;  // =  -Q*alpha
            _a[0]=	1.0;
            _a[1]=	(-2.0*co)    /a0;
            _a[2]=	(1.0 - alpha)/a0;
			}
			break;
			
		case BPF: {
			float a0= 1.0 + alpha;
            _b[0]=	  alpha      /a0;
            _b[1]=	0;
            _b[2]=	(-alpha)     /a0;
            _a[0]=	1.0;
            _a[1]=	(-2.0*co)    /a0;
            _a[2]=	(1.0 - alpha)/a0;
			}			
			break;
			
		case NOTCH: {
			float a0= 1.0 + alpha;
            _b[0]=	1.0          /a0;
            _b[1]=	(-2.0*co)    /a0;
            _b[2]=	(1.0)        /a0;
            _a[0]=	1.0;
            _a[1]=	(-2.0*co)    /a0;
            _a[2]=	(1.0 - alpha)/a0;
			}			
			break;
			
		case APF: {
			float a0= 1.0 + alpha; 
            _b[0]=	(1.0 - alpha)/a0;
            _b[1]=	(-2.0*co)    /a0;
            _b[2]=	(1.0 + alpha)/a0;
            _a[0]=	1;
            _a[1]=	(-2.0*co)    /a0;
            _a[2]=	(1.0 - alpha)/a0;
			}
			break;

		case PEAKINGEQ: {
			float A=	pow(10.0, (dBgain/40.0));
			float a0= 1.0 + alpha/A;
            _b[0]=	(1.0 + alpha*A)/a0;
            _b[1]=	(-2.0*co)      /a0;
            _b[2]=	(1.0 - alpha*A)/a0;
            _a[0]=	1.0;
            _a[1]=	(-2.0*co)      /a0;
            _a[2]=	(1.0 - alpha/A)/a0;
			}
			break;
	}
}
