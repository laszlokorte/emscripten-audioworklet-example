
#!/usr/bin/env sh

mkdir -p build
emcc main.c \
  -O2 \
  -s WASM=1 \
  -s IMPORTED_MEMORY=1 \
  -s SHARED_MEMORY=1 \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap", "HEAPF32","wasmMemory"]' \
  -o build/main.js
