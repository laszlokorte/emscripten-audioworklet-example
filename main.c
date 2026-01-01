#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

#include <emscripten/heap.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <math.h>
#define SAMPLE_RATE 44100
float phase = 0.0f;

EMSCRIPTEN_KEEPALIVE
float *generate_block(float freq, float *block_base, int block_size) {
  const float step = 2.0f * M_PI * freq / SAMPLE_RATE;

  for (int i = 0; i < block_size; i++) {
    block_base[i] = sinf(phase);
    phase += step;
    if (phase > 2.0f * M_PI)
      phase -= 2.0f * M_PI;
  }
  return block_base;
}
