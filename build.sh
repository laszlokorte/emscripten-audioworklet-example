#!/usr/bin/env sh

mkdir -p build

clang \
  --target=wasm32 \
  -O3 \
  -nostdlib \
  -matomics \
  -Wl,--no-entry \
  -Wl,--export-all \
  -Wl,--import-memory \
  -Wl,--shared-memory \
  -Wl,--max-memory=131072\
  -o build/main.wasm \
  main.c
