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
