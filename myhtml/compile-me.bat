REM emcc add.c -o js_plumbing.js -s EXTRA_EXPORTED_RUNTIME_METHODS=['ccall','cwrap']

REM em++ add.cpp  -O3 -o js_plumbing.js -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8 -s PROXY_TO_PTHREAD -s  EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -std=c++11
em++ add.cpp  -O3 -o js_plumbing.js -s ALLOW_MEMORY_GROWTH=1  -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']
