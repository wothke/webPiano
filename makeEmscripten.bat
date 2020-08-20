emcc.bat   -s WASM=1 -funroll-loops  -Os -O2 -s ASSERTIONS=0 -s SAFE_HEAP=0 -s VERBOSE=0 -fomit-frame-pointer -fno-rtti -fno-exceptions -Wno-pointer-sign --closure 1 --llvm-lto 1 -I./src  --memory-init-file 0  -s NO_FILESYSTEM=1  src/NoteUtil.cpp src/PianoVoice.cpp src/Emscripten.cpp src/Hammer.cpp src/Filter.cpp src/BiquadFilter.cpp src/PianoString.cpp src/BanksFilters.cpp -s EXPORTED_FUNCTIONS="['_initPiano', '_strikeNote', '_getSampleRate', '_getSoundBuffer', '_getSoundBufferLen', '_computeAudioSamples', '_getNumberTraceStreams', '_getTraceStreams', '_setHammerTweaks', '_setBridgeTweaks', '_setStringTweaks', '_malloc', '_free']" -o htdocs/piano.js -s SINGLE_FILE=0 -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']  -s BINARYEN_ASYNC_COMPILATION=1 -s BINARYEN_TRAP_MODE='clamp' && copy /b shell-pre.js + htdocs\piano.js + shell-post.js htdocs\piano3.js && del htdocs\piano.js && copy /b htdocs\piano3.js + piano_adapter.js htdocs\backend_piano.js && del htdocs\piano3.js
::emcc.bat -s WASM=1 -funroll-loops -g3 -s ASSERTIONS=0 -s SAFE_HEAP=0 -s VERBOSE=0 -fomit-frame-pointer -fno-rtti -fno-exceptions -Wno-pointer-sign -I./src  --memory-init-file 0  -s NO_FILESYSTEM=1  src/NoteUtil.cpp src/PianoVoice.cpp src/Emscripten.cpp src/Hammer.cpp src/Filter.cpp src/BiquadFilter.cpp src/PianoString.cpp src/BanksFilters.cpp -s EXPORTED_FUNCTIONS="['_initPiano', '_strikeNote', '_getSampleRate', '_getSoundBuffer', '_getSoundBufferLen', '_computeAudioSamples', '_getNumberTraceStreams', '_getTraceStreams', '_setHammerTweaks', '_setBridgeTweaks', '_setStringTweaks', '_malloc', '_free']" -o htdocs/piano.js -s SINGLE_FILE=0 -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']  -s BINARYEN_ASYNC_COMPILATION=1 -s BINARYEN_TRAP_MODE='clamp' && copy /b shell-pre.js + htdocs\piano.js + shell-post.js htdocs\piano3.js && del htdocs\piano.js && copy /b htdocs\piano3.js + piano_adapter.js htdocs\backend_piano.js && del htdocs\piano3.js
