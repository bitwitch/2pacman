@echo off
mkdir .\build-windows
pushd .\build-windows

xcopy /D ..\SDL2\bin\SDL2.dll .\SDL2.dll
xcopy /E /I /D ..\assets .\assets

setlocal EnableDelayedExpansion
setlocal EnableExtensions
set sources=
for %%i in (..\source\*) do set sources=!sources! %%i

gcc -Wall -g -std=gnu11 %sources% -I..\SDL2\include -I..\include -L..\SDL2\lib -lmingw32 -lSDL2main -lSDL2 -o 2pacman

popd
