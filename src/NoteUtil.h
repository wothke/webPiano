/*
* Utilites for dealing with piano notes.
*
* (c) 2020 Jürgen Wothke
* 
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
#ifndef NOTEUTIL_H
#define NOTEUTIL_H


// lowest piano note available on a regular piano
#define MIDI_OFFSET ((int)21)

// i.e. 1.0594630943592952645618252949463
#define NOTEUP ((double)pow(2.0,1.0/12.0)) 	

#define LOWEST_FREQ ((float)27.5)

/*
* Makes sure the note is in the range of regular piano keys.
*/
extern int sanitizeNote(int midiNote);

/*
* Gets relative position (range [0.0 - 1.0]) of one of the 88 piano keys. 
* @param midiNote	midi-code of a note
*/
extern float getLinearPos(int midiNote);

/*
* Gets the frequency of a note.
*/
extern float getFrequency(int midiNote);



#endif