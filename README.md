# raycast
Raycasting in C

WASD to move, left and right arrow keys to rotate

Space to shoot, r to reload

Mouse is also supported if arrow keys are too clunky for you, click to shoot

Deez nuts sprites are ammunition, the game ends when shrek touches you.

![image](https://user-images.githubusercontent.com/73869536/134826217-95698ff2-2766-49fc-afc5-77347704a68b.png)

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

