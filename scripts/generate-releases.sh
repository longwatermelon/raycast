#!/bin/bash

# Run from project root directory

read -p 'All unstaged changes will be discarded, continue? [y/n] ' yn

make_tarball() {
    cmake --build build
    cd releases
    mkdir raycast
    cd raycast

    cp ../../build/raycast .
    cp ../../map .
    cp ../../scripts/controls.txt .
    cp -r ../../res/ .

    cd ..
    tar czf $1 raycast
    rm -rf raycast
    cd ..
}

if [ "$yn" = "y" ]
then
    echo 'Continuing.'

    mkdir releases
    make_tarball raycast.tar.gz

    echo "target_compile_options(raycast PRIVATE -DCHEATS_ON)" >> CMakeLists.txt
    make_tarball raycast-cheats.tar.gz

    git restore .
else
    echo 'Aborting.'
fi;

