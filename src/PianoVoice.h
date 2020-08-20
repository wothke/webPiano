/*
* Piano synthesis based on "Physics-Based Sound Synthesis of the Piano" (Balázs Bank)
* see http://home.mit.bme.hu/~bank/thesis/pianomod.pdf
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#ifndef PIANOVOICE_H
#define PIANOVOICE_H

#include "BanksFilters.h"
#include "Hammer.h"
#include "PianoString.h"

/*
* One voice piano - sequentially plays notes, exactely one note at a time.
*
* In order to play multiple notes simultaneously multiple
* PianoVoice instances must be used. (The simulation does not
* handle any effects that the strings of different notes
* might have on each other, e.g. via the bridge.)
*
* Known limitation: There are no dampers.
*/
class PianoVoice {
public:
	PianoVoice( float sampleRate);
	~PianoVoice();
	
	void strikeNote(int midiNote, float velocity);
	long renderOutput(float *out, int samples);

	
	// for external tweaking
	void setHammerTweaks(float zh, float vh);
	void setBridgeTweaks(float g, float decay, float freq, float q, float bridgeZ);
	void setStringTweaks(float len, float thickness, float b, float sumZ, float detune, bool propagate);

private:
	float getStringVelocity();
private:

	AbstractHammer*		_hammer;	
	SoundboardFilter*	_soundboardFilter;
		
	int					_n;				// number of strings used for the played note
	PianoString*		_string[3];		// config for the used strings (up to 3)
};

#endif
