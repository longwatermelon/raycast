# raycast
Raycasting in C

Supported on linux and mac, not on windows.

Press esc to release mouse from the window and click to recapture the mouse.

The objective of the game is to collect 5 nuts, if you touch shrek your health decreases. When your health drops to 0, the game ends.

# Controls
* WASD: movement
* Mouse movement / left & right arrow keys: rotate
* Left mouse button / space: attack
* Right mouse button: grappling hook
* R: reload (gun)
* 1: equip knife
* 2: equip gun

The gun requires accuracy (red dot is crosshair) but the knife does not.

https://user-images.githubusercontent.com/73869536/140661720-0300bc67-0f27-4133-80e3-cbf32d80cd70.mp4

# Building

```
git clone https://github.com/longwatermelon/raycast
cd raycast
make
./raycast
```

The release generating shell script doesn't work with makefiles, I'll fix it later.

