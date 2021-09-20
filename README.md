# raycast
Raycasting in C

Yes, making all the sines and tangents in my code negative was intentional. Because Y increases downwards instead of upwards, if I used sin or tan normally the angle would be increasing in a clockwise direction instead of counterclockwise.

There is a memory leak in common.c when I call the getline function, but I don't know how to fix it because I'm not familiar with how I'm actually supposed to use the function.

![image](https://user-images.githubusercontent.com/73869536/133942257-c6592fcd-91f0-4c83-95f9-b0f13a98dfbe.png)

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
