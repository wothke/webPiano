/*
 piano_adapter.js: experimenting with "waveguide" based piano synth.
 
 version 0.01
 
 	Copyright (C) 2020 Juergen Wothke

 LICENSE
 
 This software is licensed under a CC BY-NC-SA 
 (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/
PianoBackendAdapter = (function(){ var $this = function (nextFrameCB) { 
		$this.base.call(this, backend_PIANO.Module, 1);	// mono
		this.playerSampleRate;
		
		this._scopeEnabled= false;	// XXX FIXME unused..
		
		this._nextFrameCB= (typeof nextFrameCB == 'undefined') ? this.nopCB : nextFrameCB;		
	}; 
	// sample buffer contains 4-byte (float) sample data
	// for 1 channel
	extend(EmsHEAPF32BackendAdapter, $this, {
		nopCB: function() {
		},
		updateSongInfo: function(filename, result) {
		},
		enableScope: function(enable) {
			this._scopeEnabled= enable;
		},		
		getAudioBuffer: function() {
			var ptr=  this.Module.ccall('getSoundBuffer', 'number');			
			return ptr>>2;	// 32 bit samples (floats)		
		},
		getAudioBufferLength: function() {
			var len= this.Module.ccall('getSoundBufferLen', 'number');
			return len;
		},
		computeAudioSamples: function() {
			// hack: buffer size is adjusted for 50fps.. i.e. 20ms data per computeAudioSamples call
			this._nextFrameCB(this);	// used for "interactive mode"
			
			var len= this.Module.ccall('computeAudioSamples', 'number');
			if (len <= 0) {
				return 1; // >0 means "end song"
			}
			
			return 0;	
		},
		getPathAndFilename: function(filename) {
			return ['/', filename];
		},
		registerFileData: function(pathFilenameArray, data) {
			return 0;	// FS not used 
		},
		loadMusicData: function(sampleRate, path, filename, data, options) {
			// try to use native sample rate to avoid resampling
			this.playerSampleRate= (typeof window._gPlayerAudioCtx == 'undefined') ? 0 : window._gPlayerAudioCtx.sampleRate;	
			var ret = this.Module.ccall('initPiano', 'number');

			if (ret == 0) {
				this.playerSampleRate = this.Module.ccall('getSampleRate', 'number');
				this.resetSampleRate(sampleRate, this.playerSampleRate); 
			}
			return 0;			
		},
		evalTrackOptions: function(options) {
			// unused
			return 0;
		},
		teardown: function() {
			// nothing to do
		},
		
		strikeNote: function(voice, note, v) {
			this.Module.ccall('strikeNote', 'number', ['number','number','number'], [voice, note, v]);			
		},
				
		setHammerTweaks: function(zh, vh) {
			this.Module.ccall('setHammerTweaks', 'number', ['number','number'], [zh, vh]);			
		},
		setBridgeTweaks: function(g, decay, freq, q, bridgeZ) {
			this.Module.ccall('setBridgeTweaks', 'number', ['number','number','number','number','number'], [g, decay, freq, q, bridgeZ]);			
		},
		setStringTweaks: function(len, thickness, b, sumZ, detune, propagate) {
			this.Module.ccall('setStringTweaks', 'number', ['number','number','number','number','number','number'], [len, thickness, b, sumZ, detune, propagate]);			
		},
		getNumberTraceStreams: function() {
			return this.Module.ccall('getNumberTraceStreams', 'number');			
		},
		getTraceStreams: function() {
			var result= [];
			var n= this.getNumberTraceStreams();

			var ret = this.Module.ccall('getTraceStreams', 'number');			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+n);
			
			for (var i= 0; i<n; i++) {
				result.push(array[i]>>1);	// pointer to int16 array
			}
			return result;
		},
		readFloatTrace: function(buffer, idx) {
			return (this.Module.HEAP16[buffer+idx])/0x8000;
		},

	});	return $this; })();
	