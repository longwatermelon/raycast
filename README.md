# raycast
Raycasting in C

Supported on linux and mac, not on windows.

Press esc to release mouse from the window and click to recapture the mouse.

The objective of the game is to collect 5 nuts, if you touch shrek your health decreases. When your health drops to 0, the game ends.

If you want to mute certain parts of the audio playing, create a file called `sfx_mute` in the same directory as where you are running the program from, and enter the following.

* `music`: mute music
* `sound`: mute sound effects
* `all`: mute all audio

# Controls
* WASD: movement
* Mouse movement / left & right arrow keys: rotate
* Left mouse button / space: attack
* Right mouse button: grappling hook
* R: reload (gun)
* 1: equip knife
* 2: equip gun

https://user-images.githubusercontent.com/73869536/143689375-5fcc2022-157c-4527-9c2c-f18d77372cb9.mp4

# Dependencies

* SDL2
* SDL2_image
* SDL2_ttf
* SDL2_mixer

Debian based: `sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev`

Arch based: `sudo pacman -S sdl2 sdl2-image sdl2-ttf sdl2-mixer`

Brew: `brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer`

# Building

```
git clone https://github.com/longwatermelon/raycast
cd raycast
make
./raycast
```

Building may fail on macos because homebrew isn't included in default library and include search paths, it should work if you add `-I[homebrew directory]/include` to INC and `-L[homebrew directory]/lib` to LIBS in the makefile. Apple clang will also spit out a lot of warnings that don't happen with gcc, so you will need to remove `-Werror` from CFLAGS.

Modified Makefile variables on an apple machine may look something like

```
CC=gcc
CFLAGS=-std=gnu17 -ggdb -Wall
INC=-I/opt/homebrew/include
LIBS=-L/opt/homebrew/lib -lm -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
```
