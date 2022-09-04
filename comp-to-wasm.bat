

REM call "C:\Dev\emsdk\emsdk_env.bat"

REM em++  hello.cpp -O3 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8  -s EXPORTED_FUNCTIONS=_int_sqrt -s EXPORTED_RUNTIME_METHODS=ccall,cwrap -o hello2.html   --shell-file html_template/shell_minimal.html 

em++  hello.cpp -O3 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8 -std=c++11 -o hello2.html   --shell-file html_template/shell_minimal.html 
