@echo off
mkdir .\build-windows
pushd .\build-windows

xcopy /D ..\SDL2\bin\SDL2.dll .\SDL2.dll
xcopy /D ..\SDL2\bin\SDL2_mixer.dll .\SDL2_mixer.dll
xcopy /D ..\SDL2\bin\libvorbis-0.dll .\libvorbis-0.dll
xcopy /D ..\SDL2\bin\libvorbisfile-3.dll .\libvorbisfile-3.dll
xcopy /D ..\SDL2\bin\libogg-0.dll .\libogg-0.dll
xcopy /E /I /D ..\assets .\assets

setlocal EnableDelayedExpansion
setlocal EnableExtensions
set sources=
for %%i in (..\source\*) do set sources=!sources! %%i

gcc -Wall -g -std=gnu11 %sources% -I..\SDL2\include -I.. -I..\include -L..\SDL2\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_mixer -o 2pacman

popd
