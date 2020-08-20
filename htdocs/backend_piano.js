// "fix-chrome-consolelog" (or else some idiot chrome versions may crash with "Illegal invokation")
(function(){ var c = window.console.log; window.console.log = function() {c.apply(window.console, arguments); };
window.printErr= window.console.log;
})();

// create separate namespace for all the Emscripten stuff.. otherwise naming clashes may occur especially when 
// optimizing using closure compiler..

window.spp_backend_state_PIANO= {
	notReady: true,
	adapterCallback: function(){}	// overwritten later	
};
window.spp_backend_state_PIANO["onRuntimeInitialized"] = function() {	// emscripten callback needed in case async init is used (e.g. for WASM)
	this.notReady= false;
	this.adapterCallback();
}.bind(window.spp_backend_state_PIANO);

var backend_PIANO = (function(Module) {
var a;a||(a=typeof Module !== 'undefined' ? Module : {});var l={},m;for(m in a)a.hasOwnProperty(m)&&(l[m]=a[m]);a.arguments=[];a.thisProgram="./this.program";a.quit=function(b,d){throw d;};a.preRun=[];a.postRun=[];var n=!1,p=!1,q=!1,r=!1;n="object"===typeof window;p="function"===typeof importScripts;q="object"===typeof process&&"function"===typeof require&&!n&&!p;r=!n&&!q&&!p;var t="";function u(b){return a.locateFile?a.locateFile(b,t):t+b}
if(q){t=__dirname+"/";var v,w;a.read=function(b,d){v||(v=require("fs"));w||(w=require("path"));b=w.normalize(b);b=v.readFileSync(b);return d?b:b.toString()};a.readBinary=function(b){b=a.read(b,!0);b.buffer||(b=new Uint8Array(b));assert(b.buffer);return b};1<process.argv.length&&(a.thisProgram=process.argv[1].replace(/\\/g,"/"));a.arguments=process.argv.slice(2);"undefined"!==typeof module&&(module.exports=a);process.on("uncaughtException",function(b){throw b;});process.on("unhandledRejection",x);
a.quit=function(b){process.exit(b)};a.inspect=function(){return"[Emscripten Module object]"}}else if(r)"undefined"!=typeof read&&(a.read=function(b){return read(b)}),a.readBinary=function(b){if("function"===typeof readbuffer)return new Uint8Array(readbuffer(b));b=read(b,"binary");assert("object"===typeof b);return b},"undefined"!=typeof scriptArgs?a.arguments=scriptArgs:"undefined"!=typeof arguments&&(a.arguments=arguments),"function"===typeof quit&&(a.quit=function(b){quit(b)});else if(n||p)p?t=
self.location.href:document.currentScript&&(t=document.currentScript.src),t=0!==t.indexOf("blob:")?t.substr(0,t.lastIndexOf("/")+1):"",a.read=function(b){var d=new XMLHttpRequest;d.open("GET",b,!1);d.send(null);return d.responseText},p&&(a.readBinary=function(b){var d=new XMLHttpRequest;d.open("GET",b,!1);d.responseType="arraybuffer";d.send(null);return new Uint8Array(d.response)}),a.readAsync=function(b,d,e){var c=new XMLHttpRequest;c.open("GET",b,!0);c.responseType="arraybuffer";c.onload=function(){200==
c.status||0==c.status&&c.response?d(c.response):e()};c.onerror=e;c.send(null)},a.setWindowTitle=function(b){document.title=b};var y=a.print||("undefined"!==typeof console?console.log.bind(console):"undefined"!==typeof print?print:null),z=a.printErr||("undefined"!==typeof printErr?printErr:"undefined"!==typeof console&&console.warn.bind(console)||y);for(m in l)l.hasOwnProperty(m)&&(a[m]=l[m]);l=void 0;function A(b){var d;d||(d=16);return Math.ceil(b/d)*d}
var aa={"f64-rem":function(b,d){return b%d},"debugger":function(){debugger}},B=!1;function assert(b,d){b||x("Assertion failed: "+d)}
var H={stackSave:function(){C()},stackRestore:function(){D()},arrayToC:function(b){var d=E(b.length);F.set(b,d);return d},stringToC:function(b){var d=0;if(null!==b&&void 0!==b&&0!==b){var e=(b.length<<2)+1;d=E(e);var c=d,g=G;if(0<e){e=c+e-1;for(var h=0;h<b.length;++h){var f=b.charCodeAt(h);if(55296<=f&&57343>=f){var k=b.charCodeAt(++h);f=65536+((f&1023)<<10)|k&1023}if(127>=f){if(c>=e)break;g[c++]=f}else{if(2047>=f){if(c+1>=e)break;g[c++]=192|f>>6}else{if(65535>=f){if(c+2>=e)break;g[c++]=224|f>>12}else{if(2097151>=
f){if(c+3>=e)break;g[c++]=240|f>>18}else{if(67108863>=f){if(c+4>=e)break;g[c++]=248|f>>24}else{if(c+5>=e)break;g[c++]=252|f>>30;g[c++]=128|f>>24&63}g[c++]=128|f>>18&63}g[c++]=128|f>>12&63}g[c++]=128|f>>6&63}g[c++]=128|f&63}}g[c]=0}}return d}},ba={string:H.stringToC,array:H.arrayToC};
function ca(b){var d;if(0===d||!b)return"";for(var e=0,c,g=0;;){c=G[b+g>>0];e|=c;if(0==c&&!d)break;g++;if(d&&g==d)break}d||(d=g);c="";if(128>e){for(;0<d;)e=String.fromCharCode.apply(String,G.subarray(b,b+Math.min(d,1024))),c=c?c+e:e,b+=1024,d-=1024;return c}a:{d=G;for(e=b;d[e];)++e;if(16<e-b&&d.subarray&&J)b=J.decode(d.subarray(b,e));else for(e="";;){c=d[b++];if(!c){b=e;break a}if(c&128)if(g=d[b++]&63,192==(c&224))e+=String.fromCharCode((c&31)<<6|g);else{var h=d[b++]&63;if(224==(c&240))c=(c&15)<<
12|g<<6|h;else{var f=d[b++]&63;if(240==(c&248))c=(c&7)<<18|g<<12|h<<6|f;else{var k=d[b++]&63;if(248==(c&252))c=(c&3)<<24|g<<18|h<<12|f<<6|k;else{var I=d[b++]&63;c=(c&1)<<30|g<<24|h<<18|f<<12|k<<6|I}}}65536>c?e+=String.fromCharCode(c):(c-=65536,e+=String.fromCharCode(55296|c>>10,56320|c&1023))}else e+=String.fromCharCode(c)}}return b}var J="undefined"!==typeof TextDecoder?new TextDecoder("utf8"):void 0;"undefined"!==typeof TextDecoder&&new TextDecoder("utf-16le");var buffer,F,G,K;
function da(){a.HEAP8=F=new Int8Array(buffer);a.HEAP16=new Int16Array(buffer);a.HEAP32=K=new Int32Array(buffer);a.HEAPU8=G=new Uint8Array(buffer);a.HEAPU16=new Uint16Array(buffer);a.HEAPU32=new Uint32Array(buffer);a.HEAPF32=new Float32Array(buffer);a.HEAPF64=new Float64Array(buffer)}var L,M,N,O,P,Q,R;L=M=N=O=P=Q=R=0;
function ea(){x("Cannot enlarge memory arrays. Either (1) compile with  -s TOTAL_MEMORY=X  with X higher than the current value "+S+", (2) compile with  -s ALLOW_MEMORY_GROWTH=1  which allows increasing the size at runtime, or (3) if you want malloc to return NULL (0) instead of this abort, compile with  -s ABORTING_MALLOC=0 ")}var T=a.TOTAL_STACK||5242880,S=a.TOTAL_MEMORY||16777216;S<T&&z("TOTAL_MEMORY should be larger than TOTAL_STACK, was "+S+"! (TOTAL_STACK="+T+")");
a.buffer?buffer=a.buffer:("object"===typeof WebAssembly&&"function"===typeof WebAssembly.Memory?(a.wasmMemory=new WebAssembly.Memory({initial:S/65536,maximum:S/65536}),buffer=a.wasmMemory.buffer):buffer=new ArrayBuffer(S),a.buffer=buffer);da();function U(b){for(;0<b.length;){var d=b.shift();if("function"==typeof d)d();else{var e=d.f;"number"===typeof e?void 0===d.a?a.dynCall_v(e):a.dynCall_vi(e,d.a):e(void 0===d.a?null:d.a)}}}var fa=[],ha=[],ia=[],ja=[],ka=!1;
function la(){var b=a.preRun.shift();fa.unshift(b)}var V=0,W=null,X=null;a.preloadedImages={};a.preloadedAudios={};function Y(b){return String.prototype.startsWith?b.startsWith("data:application/octet-stream;base64,"):0===b.indexOf("data:application/octet-stream;base64,")}
(function(){function b(){try{if(a.wasmBinary)return new Uint8Array(a.wasmBinary);if(a.readBinary)return a.readBinary(g);throw"both async and sync fetching of the wasm failed";}catch(na){x(na)}}function d(){return a.wasmBinary||!n&&!p||"function"!==typeof fetch?new Promise(function(c){c(b())}):fetch(g,{credentials:"same-origin"}).then(function(b){if(!b.ok)throw"failed to load wasm binary file at '"+g+"'";return b.arrayBuffer()}).catch(function(){return b()})}function e(b){function c(b){k=b.exports;
if(k.memory){b=k.memory;var c=a.buffer;b.byteLength<c.byteLength&&z("the new buffer in mergeMemory is smaller than the previous one. in native wasm, we should grow memory here");c=new Int8Array(c);(new Int8Array(b)).set(c);a.buffer=buffer=b;da()}a.asm=k;a.usingWasm=!0;V--;a.monitorRunDependencies&&a.monitorRunDependencies(V);0==V&&(null!==W&&(clearInterval(W),W=null),X&&(b=X,X=null,b()))}function e(b){c(b.instance)}function h(b){d().then(function(b){return WebAssembly.instantiate(b,f)}).then(b,function(b){z("failed to asynchronously prepare wasm: "+
b);x(b)})}if("object"!==typeof WebAssembly)return z("no native wasm support detected"),!1;if(!(a.wasmMemory instanceof WebAssembly.Memory))return z("no native wasm Memory in use"),!1;b.memory=a.wasmMemory;f.global={NaN:NaN,Infinity:Infinity};f["global.Math"]=Math;f.env=b;V++;a.monitorRunDependencies&&a.monitorRunDependencies(V);if(a.instantiateWasm)try{return a.instantiateWasm(f,c)}catch(oa){return z("Module.instantiateWasm callback failed with error: "+oa),!1}a.wasmBinary||"function"!==typeof WebAssembly.instantiateStreaming||
Y(g)||"function"!==typeof fetch?h(e):WebAssembly.instantiateStreaming(fetch(g,{credentials:"same-origin"}),f).then(e,function(b){z("wasm streaming compile failed: "+b);z("falling back to ArrayBuffer instantiation");h(e)});return{}}var c="piano.wast",g="piano.wasm",h="piano.temp.asm.js";Y(c)||(c=u(c));Y(g)||(g=u(g));Y(h)||(h=u(h));var f={global:null,env:null,asm2wasm:aa,parent:a},k=null;a.asmPreload=a.asm;var I=a.reallocBuffer;a.reallocBuffer=function(b){if("asmjs"===pa)var c=I(b);else a:{var d=a.usingWasm?
65536:16777216;0<b%d&&(b+=d-b%d);d=a.buffer.byteLength;if(a.usingWasm)try{c=-1!==a.wasmMemory.grow((b-d)/65536)?a.buffer=a.wasmMemory.buffer:null;break a}catch(ta){c=null;break a}c=void 0}return c};var pa="";a.asm=function(b,c){if(!c.table){b=a.wasmTableSize;void 0===b&&(b=1024);var d=a.wasmMaxTableSize;c.table="object"===typeof WebAssembly&&"function"===typeof WebAssembly.Table?void 0!==d?new WebAssembly.Table({initial:b,maximum:d,element:"anyfunc"}):new WebAssembly.Table({initial:b,element:"anyfunc"}):
Array(b);a.wasmTable=c.table}c.memoryBase||(c.memoryBase=a.STATIC_BASE);c.tableBase||(c.tableBase=0);c=e(c);assert(c,"no binaryen method succeeded.");return c}})();L=1024;M=L+18528;ha.push();a.STATIC_BASE=L;a.STATIC_BUMP=18528;M+=16;function ma(b){return Math.pow(2,b)}var qa=M;M=M+4+15&-16;R=qa;N=O=A(M);P=N+T;Q=A(P);K[R>>2]=Q;a.wasmTableSize=38;a.wasmMaxTableSize=38;a.b={};
a.c={abort:x,enlargeMemory:function(){ea()},getTotalMemory:function(){return S},abortOnCannotGrowMemory:ea,___cxa_pure_virtual:function(){B=!0;throw"Pure virtual function called!";},___setErrNo:function(b){a.___errno_location&&(K[a.___errno_location()>>2]=b);return b},_abort:function(){a.abort()},_emscripten_memcpy_big:function(b,d,e){G.set(G.subarray(d,d+e),b);return b},_llvm_exp2_f64:function(){return ma.apply(null,arguments)},_llvm_trap:function(){x("trap!")},DYNAMICTOP_PTR:R,STACKTOP:O};
var ra=a.asm(a.b,a.c,buffer);a.asm=ra;a._computeAudioSamples=function(){return a.asm._computeAudioSamples.apply(null,arguments)};a._free=function(){return a.asm._free.apply(null,arguments)};a._getNumberTraceStreams=function(){return a.asm._getNumberTraceStreams.apply(null,arguments)};a._getSampleRate=function(){return a.asm._getSampleRate.apply(null,arguments)};a._getSoundBuffer=function(){return a.asm._getSoundBuffer.apply(null,arguments)};
a._getSoundBufferLen=function(){return a.asm._getSoundBufferLen.apply(null,arguments)};a._getTraceStreams=function(){return a.asm._getTraceStreams.apply(null,arguments)};a._initPiano=function(){return a.asm._initPiano.apply(null,arguments)};a._malloc=function(){return a.asm._malloc.apply(null,arguments)};a._setBridgeTweaks=function(){return a.asm._setBridgeTweaks.apply(null,arguments)};a._setHammerTweaks=function(){return a.asm._setHammerTweaks.apply(null,arguments)};
a._setStringTweaks=function(){return a.asm._setStringTweaks.apply(null,arguments)};a._strikeNote=function(){return a.asm._strikeNote.apply(null,arguments)};var E=a.stackAlloc=function(){return a.asm.stackAlloc.apply(null,arguments)},D=a.stackRestore=function(){return a.asm.stackRestore.apply(null,arguments)},C=a.stackSave=function(){return a.asm.stackSave.apply(null,arguments)};a.dynCall_v=function(){return a.asm.dynCall_v.apply(null,arguments)};
a.dynCall_vi=function(){return a.asm.dynCall_vi.apply(null,arguments)};a.asm=ra;a.ccall=function(b,d,e,c){var g=a["_"+b];assert(g,"Cannot call unknown function "+b+", make sure it is exported");var h=[];b=0;if(c)for(var f=0;f<c.length;f++){var k=ba[e[f]];k?(0===b&&(b=C()),h[f]=k(c[f])):h[f]=c[f]}e=g.apply(null,h);e="string"===d?ca(e):"boolean"===d?!!e:e;0!==b&&D(b);return e};X=function sa(){a.calledRun||Z();a.calledRun||(X=sa)};
function Z(){function b(){if(!a.calledRun&&(a.calledRun=!0,!B)){ka||(ka=!0,U(ha));U(ia);if(a.onRuntimeInitialized)a.onRuntimeInitialized();if(a.postRun)for("function"==typeof a.postRun&&(a.postRun=[a.postRun]);a.postRun.length;){var b=a.postRun.shift();ja.unshift(b)}U(ja)}}if(!(0<V)){if(a.preRun)for("function"==typeof a.preRun&&(a.preRun=[a.preRun]);a.preRun.length;)la();U(fa);0<V||a.calledRun||(a.setStatus?(a.setStatus("Running..."),setTimeout(function(){setTimeout(function(){a.setStatus("")},1);
b()},1)):b())}}a.run=Z;function x(b){if(a.onAbort)a.onAbort(b);void 0!==b?(y(b),z(b),b=JSON.stringify(b)):b="";B=!0;throw"abort("+b+"). Build with -s ASSERTIONS=1 for more info.";}a.abort=x;if(a.preInit)for("function"==typeof a.preInit&&(a.preInit=[a.preInit]);0<a.preInit.length;)a.preInit.pop()();a.noExitRuntime=!0;Z();
  return {
	Module: Module,  // expose original Module
  };
})(window.spp_backend_state_PIANO);
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
	