# raycast
Raycasting in C

![image](https://user-images.githubusercontent.com/73869536/134788635-85a2cfbd-834e-469c-b026-07b0ab8f0cd0.png)

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
