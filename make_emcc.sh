#!/bin/sh

# based on https://lyceum-allotments.github.io/2016/06/emscripten-and-sdl2-tutorial-part-4-look-owl/
#also read https://webassembly.org/getting-started/developers-guide/
emcc ws_lesson04.c --emrun -O2 -s USE_SDL=2 -s WASM=1 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' \
	    --preload-file background.png  --preload-file image.png -o ws_lesson04.html
