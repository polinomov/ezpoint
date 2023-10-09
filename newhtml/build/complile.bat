
echo off
echo ----- compile -----
set OUT_NAME=js_gen
REM call "C:\DEV\emsdk\emsdk_env.bat"
em++  C:\DEV\ezpoint3d\newhtml\src\entry.cpp  -o %OUT_NAME%.html -s EXPORTED_FUNCTIONS=_CallCFunc,_main -s EXPORTED_RUNTIME_METHODS=ccall,cwrap

REM echo  copy %OUT_NAME%.js
REM copy /Y %OUT_NAME%.js  C:\DEV\ezpoint3d\newhtml\generated.js
REM pause
