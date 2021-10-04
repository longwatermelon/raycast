# raycast
Raycasting in C

Press esc to release mouse from the window and click to recapture the mouse.

The objective of the game is to collect 7 nuts, if you touch shrek you die.

# Controls
* WASD: movement
* Mouse movement / left & right arrow keys: rotate
* Left mouse button / space: attack
* Right mouse button: grappling hook
* R: reload (gun)
* 1: equip knife
* 2: equip gun

The gun requires accuracy (red dot is crosshair) but the knife does not.

![video](https://user-images.githubusercontent.com/73869536/135770853-ab2b7e44-722b-434a-96fb-0eb601770f44.gif)

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

