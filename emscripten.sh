#!/bin/sh
emcc -O2 -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_TTF=2 -sSDL2_IMAGE_FORMATS='["png"]' -sUSE_SDL_MIXER=2 --preload-file res -std=gnu17 src/*.c -o docs/index.html