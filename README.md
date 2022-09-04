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

Arch based: `sudo pacman -S sdl2 sdl2_image sdl2_ttf sdl2_mixer`

Brew: `brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer`

# Building

## Linux
```
git clone https://github.com/longwatermelon/raycast
cd raycast
make
./a.out
```

## MacOS
```
git clone https://github.com/longwatermelon/rasterize
cd rasterize
make INCLUDE=-I$(brew --prefix)/include LIBRARIES=-L$(brew --prefix)/lib FLAGS=-Wno-error=unused-command-line-argument
./a.out
```

# Building options
To enable cheats, add `-DCHEATS_ON` to the Makefile.

To enable map randomization, add `-DRANDOMIZE_MAP` to the Makefile.

To enable grappling hook spin, add `-DGRAPPLE_SPIN` to the Makefile.

