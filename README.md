# raycast
Raycasting in C

Press esc to release mouse and click on the screen to recapture the mouse.

WASD to move, left and right arrow keys to rotate

Space to shoot, r to reload

Mouse is also supported if arrow keys are too clunky for you, click to shoot

Press 1 to equip gun and 2 to equip knife

Deez nuts sprites are ammunition, the game ends when shrek touches you.

![image](https://user-images.githubusercontent.com/73869536/134826217-95698ff2-2766-49fc-afc5-77347704a68b.png)

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

