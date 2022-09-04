

Run python:
python -m http.server

or

python "C:\Dev\Test\HelloWorldCpp\python-server\run-python-server.py"
and run :http://127.0.0.1:8003/hello2.html


compile:
em++  hello.cpp -O3 -s ALLOW_MEMORY_GROWTH=1 -o hello2.html

em++  hello.cpp -O3 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8  -o hello2.html

em++  hello.cpp -O3 -s ALLOW_MEMORY_GROWTH=1 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=8  -o hello2.html   --shell-file html_template/shell_minimal.html

Open browser :
http://127.0.0.1:8000/


"C:\Program Files\Google\Chrome\Application\chrome.exe" --profile-directory="Default enable-features=SharedArrayBuffer

setx GIT_SSH "C:\Program Files\Git\usr\bin\ssh.exe"