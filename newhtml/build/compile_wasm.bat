
echo off
echo ----- call em++ -----
set OUT_NAME=js_gen
REM em++  C:\DEV\ezpoint3d\newhtml\src\entry.cpp  -o %OUT_NAME%.html -s WASM=1 -s EXPORTED_FUNCTIONS=_CallCFunc,_main -s ALLOW_MEMORY_GROWTH=1  -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1 -s USE_SDL=2 -s EXPORTED_RUNTIME_METHODS=ccall,cwrap

 emcc C:\DEV\ezpoint3d\newhtml\src\main.c -s WASM=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2  -s EXPORTED_FUNCTIONS=_CallCFunc,_main -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -o index.js