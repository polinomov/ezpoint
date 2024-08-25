EMCC = emcc
SOURCES = src\main.cpp \
		src\buffer.cpp \
		src\scene.cpp \
		src\camera.cpp \
		src\rdhelpers.cpp \
		src\readers\test.cpp \
		src\readers\las.cpp \
		src\chunk.cpp \

EXP_FUNC = _CallCFunc,_CallCFunc2,_main,_malloc,_free,_OnDebugCheckBox,_OnTestJS,_CameraRotateJS,_CameraMoveJS,_CameraMoveDbClickJS,_OnUIChangeJS,_LdLasCPP,_LoadFileDataJS
#EMFLAGS = -O3 -s DISABLE_EXCEPTION_CATCHING=1  -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s TOTAL_MEMORY=3gb  -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS -s EXPORTED_FUNCTIONS=$(EXP_FUNC) -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -std=c++11 -msimd128 -msse -msse2 -msse3 --bind -s FETCH=1 -s ASYNCIFY
#EMFLAGS = -O3 -s DISABLE_EXCEPTION_CATCHING=1  -s WASM=1  -s USE_SDL_IMAGE=2 -s   -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS -s EXPORTED_FUNCTIONS=$(EXP_FUNC) -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -std=c++11 -msimd128 -msse -msse2 -msse3 --bind -s FETCH=1 -s ASYNCIFY
EMFLAGS = -O3 -s DISABLE_EXCEPTION_CATCHING=1  -s WASM=1  -s USE_SDL_IMAGE=2 -s   -s ALLOW_MEMORY_GROWTH=1 -s ASSERTIONS -s EXPORTED_FUNCTIONS=$(EXP_FUNC) -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -std=c++17 -msimd128 -msse -msse2 -msse3 --bind -s FETCH=1 

build: $(SOURCES)
	$(EMCC) $(EMFLAGS) $(SOURCES) -o index.js

 