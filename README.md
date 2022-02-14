# 2pacman

I have been programming in c more lately so I wanted to write a game to  
practice and hopefully learn some things. I decided to make a pacman clone.  

## Resources
[The Pac-Man Dossier](https://pacman.holenet.info/) is amazing, it has everything you need to know about pacman.  
[Understanding Pac-Man Ghost Behavior](https://gameinternals.com/understanding-pac-man-ghost-behavior) is a nice condensed article based on The Pac-Man Dossier.  

## Dependencies
[SDL2](https://www.libsdl.org/download-2.0.php)  

# Building
## Linux
1. Install SDL2, linux instructions [here](http://wiki.libsdl.org/Installation#linuxunix)  
2. run `make`
3. run `./2pacman`

## Windows
1. Install mingw-w64 
  - You can find the specific version you want from [here](https://sourceforge.net/projects/mingw-w64/files/).  
  - The verision I used was version 8.1.0, [x86_64-posix-seh](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z)  
  - Add the mingw-w64 bin directory to your path environment variable, so that you can use gcc  
  - You can find more details about setting up SDL2 with mingw-w64 [here](https://www.matsson.com/prog/sdl2-mingw-w64-tutorial.php).  
2. Install SDL2
  - Download the [mingw-w64 development libraries](https://www.libsdl.org/release/SDL2-devel-2.0.20-mingw.tar.gz)  
  - After extracting the contents, copy the directory `x86_64-w64-mingw32` to a  
    directory called `SDL2` in this project    
3. run `build`  

## macOS
Not tested yet, though should work similar to Linux. Probably using clang  
instead of gcc.   

