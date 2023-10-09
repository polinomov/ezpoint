@echo off
echo -------- build ----------------
call C:\DEV\emsdk\emsdk_env.bat
echo  -------- call power shell-------------
powershell -executionpolicy remotesigned -File build.ps1