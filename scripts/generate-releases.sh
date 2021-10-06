#!/bin/bash

# Run from project root directory

echo "If you haven't commited your latest changes, commit now."
read -p 'Continue? [y/n] ' yn

build_raycast() {
    cmake --build build
    cd releases
    mkdir raycast
    cp ../build/raycast raycast/
    cp ../map raycast/
    cp -r ../res/ raycast/
}

if [ "$yn" = "y" ]
then
    echo 'Continuing.'

    mkdir releases

    build_raycast
    tar czf raycast.tar.gz raycast
    rm -rf raycast
    cd ..

    echo "target_compile_options(raycast PRIVATE -DCHEATS_ON)" >> CMakeLists.txt
    cmake --build build

    build_raycast
    cp ../scripts/controls.txt raycast/
    tar czf raycast-cheats.tar.gz raycast
    rm -rf raycast
    cd ..

    git restore .
else
    echo 'Aborting.'
fi;

