/*
* Generic filters used in "Physics-Based Sound Synthesis of the Piano" (Balázs Bank)
* see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#ifndef FILTER_H
#define FILTER_H

/**
* Generic filter interface.
*/
class IFilter {
public:
	virtual float process(float input)= 0;	
	virtual float getPhaseDelay(float freq, float sampleRate)= 0;
	virtual float getGroupDelay(float freq, float sampleRate)= 0;
};

/*
* Delays a signal by a fixed number of samples.
*/
class DelayFilter : public IFilter {
public:
	DelayFilter();
	virtual ~DelayFilter();

	void init(int delay);
	
	virtual float process(float input);	
	virtual float getPhaseDelay(float freq, float sampleRate);
	virtual float getGroupDelay(float freq, float sampleRate);
private:
	int next_pow2(int x);
	float readNext();
	void writeNext(float input);

private:
	int _delay;

	float *_circularBuffer;
	int _size;	// power of 2 to ease overflow handling
	int _overflowMask;

	int _readIdx;
	int _writeIdx;
};


/**
* Base for IIR filters.
*
* note: coefficents must be normalized to a[0]= 1 
*/
class IIRFilter {
public:
	IIRFilter();
	virtual ~IIRFilter();
	
	float process(float input);
	float getPhaseDelay(float freq, float sampleRate);
	float getGroupDelay(float freq, float sampleRate);

	int getOrder();

private:
	void shiftBufferRight(float *buf);
protected:
	virtual void orderInit(int n);
protected:
	float *_x;	// past inputs 
	float *_y;	// past outputs
	
	float *_b;	// feed forward coefficiants (weights for past inputs)
	float *_a; 	// feedback coefficients (weights for past outputs)

	int _n; 	// order of the filter
	int _size;	// size of allocated buffers
};

/*
* loss filter - see thesis p.118 / 119
*/
class LossFilter : public IIRFilter {
public:
	LossFilter();
	virtual ~LossFilter();

	//  c1 and c3 coefficients correspond to the first- and third-order time derivates of the wave equation 
	void init(float freq, float c1, float c3);
};

/*
* Thiran allpass filter.
*/
class ThiranFilter : public IIRFilter {
public:
	ThiranFilter();
	virtual ~ThiranFilter();

	/*
	@param n order of the filter (N)
	@param d desired delay (D)
	*/
	void init(int n, float d);
private:
	double nChooseK(int n, int k);
};



#endif