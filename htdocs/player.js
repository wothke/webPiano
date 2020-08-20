/**
* Facade used to wrap the ScriptNodePlayer emulator plugin.
*
* <p>This is not meant to be reusable but it might give an idea how the ScriptNodePlayer is used.
*/

class Player {
	constructor(playingCallback, nextFrameCallback) {
		this._doParseUrl= 	function(url) {	
				var options= {};
				options.track= 1;
				options.timeout= -1;
				options.traceSID= true;	// enable additional output used by the SidTracer
									
				return [url, options];
			};
			
		var bufferSize= 16384>>2;
		var streams= 1;
		window.sidTracer= new Tracer(bufferSize, streams);	// buffer set later depending on zoom
						
		var doOnTrackReadyToPlay= function(){ 	
			ScriptNodePlayer.getInstance().play();
			playingCallback();
		}
			
		// --------------------------- SID music player -----------------------
		this.backend= new PianoBackendAdapter(nextFrameCallback);
		ScriptNodePlayer.createInstance(this.backend, "", [], false, this.doOnPlayerReady.bind(this),
										doOnTrackReadyToPlay, this.doOnTrackEnd.bind(this), function(){}, sidTracer, bufferSize);
			
		ScriptNodePlayer.getInstance().setSilenceTimeout(0);
		
		// depending on the browser/timing the player may be ready before or after (see WASM) init(), i.e.
		// the startup sequence must handle both cases (music can only be started when
		// both the player is ready and init() has been completed..)
		this.playerReady= false;		
	}
	getBackend() {
		return this.backend;
	}
	doOnTrackEnd(){
//		if (player) {
			var url= '';	// not needed here			
			this.playSong(url);
//		}
	}

	checkReady() {
		if (this.playerReady) {
			this.doOnTrackEnd(); // player was ready before it could trigger the playback
		}
	}
	doOnPlayerReady() {
		this.playerReady= true;
//		if (player) 
		this.doOnTrackEnd(); // player are used to check for init()
	}

	getTracer() {
		return window.sidTracer;
	}
	pause() 			{ ScriptNodePlayer.getInstance().pause(); }
	resume() 			{ ScriptNodePlayer.getInstance().resume(); }
	setVolume(value) 	{ ScriptNodePlayer.getInstance().setVolume(value); }
	getSongInfo ()		{ return ScriptNodePlayer.getInstance().getSongInfo(); }
		
	playSongWithBackand(options, onSuccess) {
		// backend adapter to be used has been explicitly specified	
		var o= options.backendAdapter;
		ScriptNodePlayer.createInstance(o.adapter, o.basePath, o.preload, o.enableSpectrum, 
										onSuccess, o.doOnTrackReadyToPlay, o.doOnTrackEnd);
	}
	playSong(someSongURL) {
		var audioCtx= ScriptNodePlayer.getInstance().getAudioContext();	// handle Google's bullshit "autoplay policy"
		if (audioCtx.state == "suspended") {
			var modal = document.getElementById('autoplayConfirm');	
			modal.style.display = "block";		// force user to click
			
			window.globalDeferredPlay = function() {	// setup function to be used "onClick"
				audioCtx.resume();
				this._playSong(someSongURL);
			}.bind(this);
			
		} else {
			this._playSong(someSongURL);
		}
	}
	_playSong(someSongURL) {
		var arr= this._doParseUrl(someSongURL);
		var options= arr[1];
		if (typeof options.backendAdapter != 'undefined') {
			var name= arr[0];
			var o= options.backendAdapter;
			this.playSongWithBackand(options, (function(){
													var p= ScriptNodePlayer.getInstance();
													
													p.loadMusicFromURL(name, options, 
														(function(filename){
														}), 
														(function(){}), 
														(function(total, loaded){}));
													
													o.doOnPlayerReady();			
												}.bind(this)));
		} else {
			var p= ScriptNodePlayer.getInstance();
			if (p.isReady()) {
				p.loadMusicFromURL(arr[0], options, 
				(function(filename){}), 
				(function(){}), 
				(function(total, loaded){}));
			}
		}
	}

};
