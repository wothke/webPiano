/*
* Utilites for dealing with piano notes.
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#include <math.h>

#include "NoteUtil.h"

int sanitizeNote(int midiNote) {
	if (midiNote < MIDI_OFFSET) midiNote= MIDI_OFFSET;	// block non-piano notes
	if (midiNote > 108) midiNote= 108;
	return midiNote;
}

float getLinearPos(int midiNote) {
	return (float)(midiNote-MIDI_OFFSET) / (108-MIDI_OFFSET);
}

float getFrequency(int midiNote) {
	// frequency doubles every octave, i.e. after 12 halftone-steps

	// lowest midi note is C with 8.175Hz, on a piano the center "Kammerton" (also called
	// a’ or A4) is 440Hz (midiNote=69) and the lowest note available on a regular 
	// piano is ,,A 27.5 Hz (which corresponds to midiNote=21) and highest midiNote=108 
	// is 4186Hz

	return pow(NOTEUP, (midiNote-69))*440.0;
}

