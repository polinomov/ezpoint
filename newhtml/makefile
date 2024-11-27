EMCC = emcc
SOURCES = src\main.cpp\
		src\buffer.cpp \
		src\scene.cpp \
		src\camera.cpp \
		src\rdhelpers.cpp \
		src\readers\test.cpp \
		src\readers\las.cpp \
		src\chunk.cpp \

EXP_FUNC = _CallCFunc,_CallCFunc2,_main,_malloc,_free,_OnDebugCheckBox,_OnTestJS,_CameraRotateJS,_CameraMoveJS,_CameraMoveDbClickJS
EXP_FUNC := $(EXP_FUNC),_OnUIChangeJS
EXP_FUNC := $(EXP_FUNC),_LdLasCppJS
EXP_FUNC := $(EXP_FUNC),_PostProcessDataJS
EXP_FUNC := $(EXP_FUNC),_MouseMoveJS
EXP_FUNC := $(EXP_FUNC),_MouseClickJS

EMFLAGS = -O3 \
-s DISABLE_EXCEPTION_CATCHING=1 \
-s WASM=1 \
-s USE_SDL_IMAGE=2 \
-s SDL2_IMAGE_FORMATS=["png"] \
-s USE_SDL_TTF=2 \
-s ALLOW_MEMORY_GROWTH=1 \
-s MAXIMUM_MEMORY=4GB \
-s ASSERTIONS \
-s EXPORTED_FUNCTIONS=$(EXP_FUNC) \
-s EXPORTED_RUNTIME_METHODS=ccall,cwrap -std=c++17 -msimd128 -msse -msse2 -msse3 --bind -s FETCH=1 

build: $(SOURCES)
	$(EMCC) $(EMFLAGS) $(SOURCES) -o index.js

 