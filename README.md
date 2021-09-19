# raycast
Raycasting in C

Yes, making all the sines and tangents in my code negative was intentional. Because Y increases downwards instead of upwards, if I used sin or tan normally the angle would be increasing in a clockwise direction instead of counterclockwise.

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
