#!/usr/bin/env sh

mkdir -p build
emcc_args=(
    -O2
    -s WASM=1
    -s IMPORTED_MEMORY=1  # use js allocated provided memory
    -s SHARED_MEMORY=1  # allow shared memory
)
emcc main.c "${emcc_args[@]}" -o build/main.js
