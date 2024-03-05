EMCC = emcc
SOURCES = src\main.cpp \
		src\buffer.cpp \
		src\scene.cpp \
		src\camera.cpp \
		src\rdhelpers.cpp \
		src\readers\test.cpp \
		src\readers\las.cpp \
		src\chunk.cpp \

EXP_FUNC = _CallCFunc,_CallCFunc2,_main,_malloc,_free,_FileBinDataJS,_OnDebugCheckBox,_OnTestJS,_CameraRotateJS,_CameraMoveJS,_OnUIChangeJS
EMFLAGS = -O3 -s DISABLE_EXCEPTION_CATCHING=1  -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s TOTAL_MEMORY=3221225472  -s ASSERTIONS -s EXPORTED_FUNCTIONS=$(EXP_FUNC) -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -std=c++11 -msimd128 -msse -msse2 -msse3

build: $(SOURCES)
	$(EMCC) $(EMFLAGS) $(SOURCES) -o index.js

 