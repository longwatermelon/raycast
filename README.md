# raycast
Raycasting in C

Press esc to release mouse from the window and click to recapture the mouse.

# Controls
* WASD: movement
* Mouse movement / left & right arrow keys: rotate
* Left mouse button / space: attack
* Right mouse button: grappling hook
* R: reload
* 1: equip gun
* 2: equip knife

The game ends when shrek touches you.

![video](https://user-images.githubusercontent.com/73869536/135743618-085a03ae-62b0-4f53-ae91-bcee3d7794bc.gif)

Anything in the code that is related to rendering and uses trig needs to make sin and tan negative, since the y coordinates are flipped in programming. Everything else still uses sin and tan normally.

# Building

You need [vcpkg](https://github.com/microsoft/vcpkg) to build this project.

```
git clone https://github.com/longwatermelon/raycast
cd raycast
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
make
cd .. && build/raycast
```

