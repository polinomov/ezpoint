EMCC = emcc
SOURCES = src\main.cpp \
		src\buffer.cpp \
		src\scene.cpp \
		src\camera.cpp \
		src\readers\test.cpp \
		src\readers\las.cpp \

EXP_FUNC = _CallCFunc,_CallCFunc2,_main,_FileBinDataJS,_OnDebugCheckBox,_OnTestJS,_CameraRotateJS
EMFLAGS = -O3 -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY -s ASYNCIFY_IMPORTS=[do_fetch] -s ASSERTIONS -s EXPORTED_FUNCTIONS=$(EXP_FUNC) -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -std=c++11 -msimd128

build: $(SOURCES)
	$(EMCC) $(EMFLAGS) $(SOURCES) -o index.js

 