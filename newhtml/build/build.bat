
echo off
echo ----- build ----------------------
set OUT_NAME=index
REM call "C:\DEV\emsdk\emsdk_env.bat"

call compile_wasm.bat

echo  copy %OUT_NAME%.js
copy /Y index.js  C:\DEV\ezpoint3d\newhtml\index.js
copy /Y index.wasm  C:\DEV\ezpoint3d\newhtml\index.wasm
