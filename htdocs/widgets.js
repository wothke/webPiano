/*
* various UI widgets used for WebPiano screen
*
* 	Copyright (C) 2020 Juergen Wothke
*/

class MusicBox {
	constructor(callbackArray) {
		this.lastTs= -1;
		//this.clock= 120;	// ms
		this.clock= 130;	// ms
		
		this.callbackArray= callbackArray;
		
		this.autoplay= false;

		this.systemTs= 0; // absolute system time in ms

		// play same melodie on all voices using an offset
		this.voiceOffset= 4*32;
		
		this.pos= [0, -this.voiceOffset, -this.voiceOffset*2];
		
		// Canon in D Johann Pachelbel
		// ref for used numbering: 0=H
		
		var song= '\
m*18*8,16*8,14*8,13*8, 11*8,9*8,11*8,13*8, 18*8,16*8,14*8,13*8, 11*8,9*8,11*8,13*8,\
2*4,6*4,9*4,7*4,6*4,2*4,6*4,4*4, 2*4,-1*4,2*4,9*4,7*4,11*4,9*4,7*4, 6*4,2*4,4*4,13*4,14*4,18*4,21*4,9*4, 11*4,7*4,9*4,6*4,2*4,14*4,14*1,16*1,14*1,16*1,14*2,13*2,\
14*2,13*2,14*2,2*2,1*2,9*2,4*2,6*2,2*2,14*2,13*2,11*2,13*2,18*2,21*2,23*2, 19*2,18*2,16*2,19*2,18*2,16*2,14*2,13*2,11*2,9*2,7*2,6*2,4*2,7*2,6*2,4*2, 2*2,4*2,6*2,7*2,9*2,4*2,9*2,7*2,6*2,11*2,9*2,7*2,9*2,7*2,6*2,4*2, 2*2,-1*2,11*2,13*2,14*2,13*2,11*2,9*2,7*2,6*2,4*2,11*2,9*2,11*2,9*2,7*2,\
6*4,18*4,16*12,14*4,18*8, 23*8,21*8,23*8,25*8, 26*4,14*4,13*12,11*4,14*8, 14*12,14*4,14*4,19*4,16*4,21*4,\
f*21*2,18*1,19*1,21*2,18*1,19*1,21*1,9*1,11*1,13*1,14*1,16*1,18*1,19*1,18*2,14*1,16*1,18*2,6*1,7*1,9*1,11*1,9*1,7*1,9*1,6*1,7*1,9*1, 7*2,11*1,9*1,7*2,6*1,4*1,6*1,4*1,2*1,4*1,6*1,7*1,9*1,11*1,7*2,11*1,9*1,11*2,13*1,14*1,9*1,11*1,13*1,14*1,16*1,18*1,19*1,21*1, 18*2,14*1,16*1,18*2,16*1,14*1,16*1,13*1,14*1,16*1,18*1,16*1,14*1,13*1,14*2,11*1,13*1,14*2,2*1,4*1,6*1,7*1,6*1,4*1,6*1,14*1,13*1,14*1, 11*2,14*1,13*1,11*2,9*1,7*1,9*1,7*1,6*1,7*1,9*1,11*1,13*1,14*1,11*2,14*1,13*1,14*2,13*1,11*1,9*1,11*1,13*1,14*1,16*1,18*1,19*1,21*1,\
14*8,9*8,11*8,6*8, 7*8,2*8,7*8,9*8, 18*8,16*8,14*8,13*8, 11*8,9*8,11*8,13*8,\
14*128\
';
		this.initNotes(song);
	}
	getForte(f) {
		if (f == "m") {	// mezzo forte
			return 2.5;
		} else if(f == "f") {	// fortissimo
			return 3.5;
		}
		console.log("ERROR: getForte(f)");
		return 1;	// error: not implemented
	}
	initNotes(song){
		var f= "m";
		var arr= song.split(",");
		this.notes= [];
		this.forte= [];
		for (var i= 0; i<arr.length; i++) {
			var n= arr[i].split("*");
			if (n.length == 3) {
				f= n.shift();
			}
			
			var note= parseInt(n[0]);
			this.notes.push(note + 1*12);	// neutralize negative numbers
			this.forte.push(this.getForte(f));
			
			var u= parseInt(n[1]) -1;
			
			for (var j= 0; j<u; j++) {
				this.notes.push(-1);	// just hold previous note
				this.forte.push(this.getForte(f));
			}					
		}
	}
	playVoice(voiceIdx) {
		var p= this.pos[voiceIdx]++;
		
		if(p>=0) {
			var t= this.notes[p];
			if (p >= this.notes.length) this.pos[voiceIdx]= -this.voiceOffset*voiceIdx;
			if (t < 0) {
				// just wait
			} else {
				var f= this.forte[p];
				this.callbackArray[voiceIdx](voiceIdx, 3+Math.floor(t/12), t%12, f);
			}
		}
	}
	play() {
		if (!this.autoplay) return false;
		
		this.systemTs+= 20;	// PAL 50Hz			

		var diff= this.systemTs - this.lastTs;
		if (diff >= this.clock) {
			this.playVoice(0);
			this.playVoice(1);
			this.playVoice(2);
						
			this.lastTs= this.systemTs;
		}
		return true;
	}
	inject(divSelect) {
		$(divSelect).html(
			'<div class="a1">\
				<div>\
					<input type="checkbox" class="regular-checkbox" id="autoplay" value="autoplay" /> autoplay\
				</div>\
			</div>'
		);
	
		var that= this;
		$('#autoplay').prop('checked', this.autoplay);		
		$('#autoplay').change(function(e) {
			that.autoplay= this.checked;
		});
	}
		
}

class WidgetPaypal {
	constructor() {
	}
	inject() {
		var tooltipDiv= document.getElementById("tooltip");

		var f = document.createElement("form");
		f.setAttribute('method',"post");
		f.setAttribute('action',"https://www.paypal.com/cgi-bin/webscr");
		f.setAttribute('target',"_blank");

		var i1 = document.createElement("input");
		i1.type = "hidden";
		i1.value = "_s-xclick";
		i1.name = "cmd";
		f.appendChild(i1);  
		
		var i2 = document.createElement("input");
		i2.type = "hidden";
		i2.value = "E7ACAHA7W5FYC";
		i2.name = "hosted_button_id";
		f.appendChild(i2);  
		
		var i3 = document.createElement("input");
		i3.type = "image";
		i3.src= "stdlib/btn_donate_LG.gif";
		i3.border= "0";
		i3.name="submit";
		i3.alt="PayPal - The safer, easier way to pay online!";
		f.appendChild(i3);  
		
		var i4 = document.createElement("img");
		i4.alt = "";
		i4.border = "0";
		i4.src = "stdlib/pixel.gif";
		i4.width = "1";
		i4.height = "1";
		f.appendChild(i4);  
		
		tooltipDiv.appendChild(f);  
	}
};

class WidgetUserEngagement {
	constructor() {
	}
	inject() {
		// handle Goggle's latest "autoplay policy" bullshit (patch the HTML/Script from here within this
		// lib so that the various existing html files need not be touched)
						
		var d = document.createElement("DIV");
		d.setAttribute("id", "autoplayConfirm");
		d.setAttribute("class", "modal-autoplay");

		var dc = document.createElement("DIV");
		dc.setAttribute("class", "modal-autoplay-content");
		
		var p = document.createElement("P");
		var t = document.createTextNode("You may thank the clueless Google Chrome idiots for this useless add-on dialog - without which their \
		user unfriendly browser will no longer play the music (which is the whole point of this page).\
		Click outside of this box to continue.");
		p.appendChild(t);
		
		dc.appendChild(p);
		d.appendChild(dc);
		
		document.body.insertBefore(d, document.body.firstChild);

		
		var s= document.createElement('script');
		s.text = 'var modal = document.getElementById("autoplayConfirm");\
			window.onclick = function(event) {\
				if (event.target == modal) {\
					modal.style.display = "none";\
					if (typeof window.globalDeferredPlay !== "undefined") { window.globalDeferredPlay();\
					delete window.globalDeferredPlay; }\
				}\
			}';
		document.body.appendChild(s);
	}
};



class WidgetBase {
	constructor() {
	}
	update(t){
		// override in sublasses
	}
	setupKnob(selector, initVal, setFunc) {
		var knob;

		var callback= function(e) {
			// get current settings
//			var v= knob.prevObject[0].value;
//			setFunc(parseInt(v));
			if (typeof e == 'undefined') return;

			setFunc(e);
			
			this.update(performance.now());					
		}.bind(this);	
		
		$(selector).val(initVal);				
		knob= $(selector).knob({ 'change': callback });
	
		callback();
	}
	setupContext(canvasId, w, h) {
		var c = document.getElementById(canvasId);
		c.width = w;
		c.height = h;
		return c;
	}
	cleanCanvas(canvas) {
		var ctx = canvas.getContext('2d');

		try {
			// seems that dumbshit Safari (11.0.1 OS X) uses the fillStyle for "clearRect"!
			ctx.fillStyle = "rgba(0, 0, 0, 0.0)";
		} catch (err) {}
		ctx.clearRect(0, 0, canvas.width, canvas.height);
		
		ctx.lineWidth = 3;
		ctx.save();
	}
	updateLine(canvas, x1, y1, x2, y2, color) {	// x range 0..1 y range 0..15
		var ctx = canvas.getContext('2d');
		var w= canvas.width;
		var h= canvas.height;
		
		ctx.beginPath();
		ctx.moveTo(w*x1, h-(h*y1/0xf));	// y axis upside down
		ctx.lineTo(w*x2, h-(h*y2/0xf));
		ctx.strokeStyle = color;
		
		ctx.stroke();
		ctx.restore();
	}
}

class WidgetStringTweaks extends WidgetBase {
	constructor() {
		super();
		this.idx= "0";
		
		this.stringLen= 1.0; 	// values < 0.3 will crash for high notes
		this.stringThickness= 1.0;		
		this.stringB= 1.0;		
		this.stringSumZ= 2.0;
		this.stringDetune= 7.0;
		this.stringPropagate= 0.0;

		this.changed= false;
	}
	updateBackend(backend) {
		if (this.changed) {
			backend.setStringTweaks(this.stringLen, this.stringThickness, this.stringB, 
									this.stringSumZ, this.stringDetune, this.stringPropagate );
			this.changed= false;
		} 
	}
	processLenDial(v) {
		if (v != this.stringLen) {
			this.stringLen= v;
			this.changed= true;
		}
	}
	processThicknessDial(v) {
		if (v != this.stringThickness) {
			this.stringThickness= v;
			this.changed= true;
		}
	}
	processBDial(v) {
		if (v != this.stringB) {
			this.stringB= v;
			this.changed= true;
		}
	}
	processSumZDial(v) {
		if (v != this.stringSumZ) {
			this.stringSumZ= v;
			this.changed= true;
		}
	}
	processDetuneDial(v) {
		if (v != this.stringDetune) {
			this.stringDetune= v;
			this.changed= true;
		}
	}
	processPropagateDial(v) {
		if (v != this.stringPropagate) {
			this.stringPropagate= v;
			this.changed= true;
		}
	}
	inject(divSelect) {
		$(divSelect).html(
			'<div class="a11" style="width:120px;"><b>string</b>\
			</div>\
			<div class="a11">len\
				<div class="knobDiv">\
					<input class="stringLen" data-width="50" data-min="0.3" data-max="2.0" data-step=".02" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">&Oslash;\
				<div class="knobDiv">\
					<input class="stringThickness" data-width="50" data-min="0.5" data-max="2.0" data-step=".02" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">B\
				<div class="knobDiv">\
					<input class="stringB" data-width="50" data-min="0.0" data-max="2.0" data-step=".02" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">&Sigma;\
				<div class="knobDiv">\
					<input class="stringSumZ" data-width="50" data-min="1.0" data-max="2.0" data-step="0.1" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">&nu;\
				<div class="knobDiv">\
					<input class="stringDetune" data-width="50" data-min="1.0" data-max="25.0" data-step="0.2" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">&Xi;\
				<div>\
					<input type="checkbox" class="regular-checkbox" id="stringPropagate" value="stringPropagate" />\
				</div>\
			</div>'
		);

		var c= function(enabled) {
			this.stringPropagate= enabled ? 1 : 0;		
			this.changed= true;
		}.bind(this);
		
		$('#stringPropagate').prop('checked', this.stringPropagate);		
		$('#stringPropagate').change(function(e) {
			c(this.checked);
		});

		this.setupKnob(".stringLen", this.stringLen, this.processLenDial.bind(this));
		this.setupKnob(".stringThickness", this.stringThickness, this.processThicknessDial.bind(this));
		this.setupKnob(".stringB", this.stringB, this.processBDial.bind(this));		
		this.setupKnob(".stringSumZ", this.stringSumZ, this.processSumZDial.bind(this));
		this.setupKnob(".stringDetune", this.stringDetune, this.processDetuneDial.bind(this));
	}
}

class WidgetSoundboardTreaks extends WidgetBase {
	constructor() {
		super();
		this.idx= "0";
		
		this.lowpassFreq= 3000; 
		this.lowpassQ= 1.207;	// allow range 0.1 - 2.1
		
		this.lossG= 0.95; 
		this.lossDecay= 1.2;

		this.bridgeZ= 1.0;
		
		this.hammerZ= 0.0;
		this.hammerV= 1.0;

		this.changed= false;
	}
	updateBackend(backend) {
		if (this.changed) {
			backend.setBridgeTweaks(this.lossG, this.lossDecay, this.lowpassFreq, this.lowpassQ, this.bridgeZ);
			backend.setHammerTweaks(this.hammerZ, this.hammerV);
			
			this.changed= false;
		} 
	}
	processZbridgeDial(v) {
		if (v != this.bridgeZ) {
			this.bridgeZ= v;
			this.changed= true;
		}
	}
	processVhammerDial(v) {
		if (v != this.hammerV) {
			this.hammerV= v;
			this.changed= true;
		}
	}
	processZhammerDial(v) {
		if (v != this.hammerZ) {
			this.hammerZ= v;
			this.changed= true;
		}
	}
	processFreqDial(v) {
		if (v != this.lowpassFreq) {
			this.lowpassFreq= v;
			this.changed= true;
		}
	}
	processQDial(v) {
		if (v != this.lowpassQ) {
			this.lowpassQ= v;
			this.changed= true;
		}
	}
	processGDial(v) {
		if (v != this.lossG) {
			this.lossG= v;
			this.changed= true;
		}
	}
	processDecayDial(v) {
		if (v != this.lossDecay) {
			this.lossDecay= v;
			this.changed= true;
		}
	}
	inject(divSelect) {
		$(divSelect).html(
			'<div class="a11" style="width:120px;"><b>soundboard</b>\
			</div>\
			<div class="a11">lowpass&nbsp;(freq/Q)\
				<div class="knobDiv">\
					<input class="lowpassFreq" data-width="50" data-min=1 data-max=5000 data-cursor=true>\
				</div>\
			</div>\
			<div class="a11">&nbsp;\
				<div class="knobDiv">\
					<input class="lowpassQ" data-width="50" data-min="0.1" data-max=3 data-step=".02" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">loss&nbsp;(G/decay)\
				<div class="knobDiv">\
					<input class="lossG" data-width="50" data-min="0.9" data-max="1.0" data-step=".01" data-cursor=true>\
				</div>\
			</div>\
			<div class="a11">&nbsp;\
				<div class="knobDiv">\
					<input class="lossDecay" data-width="50" data-min="0.5" data-max="2.0" data-step=".1" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">Z\
				<div class="knobDiv">\
					<input class="bridgeZ" data-width="50" data-min="0.5" data-max="2.0" data-step=".1" data-cursor=true>\
				</div>\
			</div>\
			<div class="a11" style="width:20px;">&nbsp;\
			</div>\
			<div class="a11" style="width:80px;"><b>hammer</b>\
			</div>\
			<div class="a11">Z\
				<div class="knobDiv">\
					<input class="hammerZ" data-width="50" data-min=0 data-max=1 data-step=".01" data-cursor=true>\
				</div>\
			</div>\
			<div class="a2">&nbsp;<span ></span></div>\
			<div class="a11">V\
				<div class="knobDiv">\
					<input class="hammerV" data-width="50" data-min=0 data-max=2 data-step=".01" data-cursor=true>\
				</div>\
			</div>'
		);

		
		this.setupKnob(".lowpassFreq", this.lowpassFreq, this.processFreqDial.bind(this));
		this.setupKnob(".lowpassQ", this.lowpassQ, this.processQDial.bind(this));
		
		this.setupKnob(".lossG", this.lossG, this.processGDial.bind(this));
		this.setupKnob(".lossDecay", this.lossDecay, this.processDecayDial.bind(this));
		this.setupKnob(".bridgeZ", this.bridgeZ, this.processZbridgeDial.bind(this));
		this.setupKnob(".hammerZ", this.hammerZ, this.processZhammerDial.bind(this));
		this.setupKnob(".hammerV", this.hammerV, this.processVhammerDial.bind(this));
	}
}
