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
#include "Filter.h"


enum BiquadType {
  LPF= 0, 			// H(s) = 1 / (s^2 + s/Q + 1)
  HPF= 1, 			// H(s) = s^2 / (s^2 + s/Q + 1)
  BPFQ= 2,			// H(s) = s / (s^2 + s/Q + 1)  (constant skirt gain, peak gain = Q)
  BPF= 3,			// H(s) = (s/Q) / (s^2 + s/Q + 1)      (constant 0 dB peak gain)
  NOTCH= 4,			// H(s) = (s^2 + 1) / (s^2 + s/Q + 1)
  APF= 5,			// H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)
  PEAKINGEQ= 6, 	// H(s) = (s^2 + s*(A/Q) + 1) / (s^2 + s/(A*Q) + 1)
};

enum BiquadTypeShelf {
  LOWSHELF= 7,		// H(s) = A * (s^2 + (sqrt(A)/Q)*s + A)/(A*s^2 + (sqrt(A)/Q)*s + 1)
  HIGHSHELF= 8		// A * (A*s^2 + (sqrt(A)/Q)*s + 1)/(s^2 + (sqrt(A)/Q)*s + A)
};

class BiquadFilter : public IIRFilter {
public:
	BiquadFilter();
	virtual ~BiquadFilter();

	/*
	@param dBgain for peaking and shelving EQ filters only
	@param Q the EE kind of definition, except for PEAKINGEQ in which A*Q is the classic EE Q
	*/
	void init(BiquadType type, float sampleRate, float freq, float Q, float dBgain= 0.0);
	void initBW(BiquadType type, float sampleRate, float freq, float BW, float dBgain= 0.0);
	void init(BiquadTypeShelf type, float sampleRate, float freq, float S, float dBgain= 0.0);
private:
	void _init(BiquadType type, float w0, float alpha, float dBgain);
};