#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include <math.h>
#include <stddef.h>
#define SAMPLE_RATE 44100
float phase = 0.0f;
int sample = 0;

typedef struct sound {
  float volume;
  float freq;
  int duration;
  int time;
  struct sound *next;
} sound;

typedef struct arena {
  unsigned int used;
  unsigned int capacity;
  unsigned char *memory;
} arena;

void *_arena_push(arena *a, size_t size) {
  if (a->used + size <= a->capacity) {
    void *result = &a->memory[a->used];

    a->used += size;
    return result;
  } else {
    return 0;
  }
}
#define arena_push(arena, type) ((type *)_arena_push(arena, sizeof(type)))
#define arena_push_array(arena, type, count)                                   \
  ((type[(count)])_arena_push((arena), ((count) * sizeof(type))))

unsigned char global_arena_memory[1000];
struct arena global_arena;

struct sound *next_sound;

EMSCRIPTEN_KEEPALIVE
float *generate_block(float freq, float *block_base, int block_size) {
  const float step = 2.0f * M_PI * freq / SAMPLE_RATE;

  for (int i = 0; i < block_size; i++) {

    block_base[i] = sinf(phase);
    phase += step;
    if (phase > 2.0f * M_PI)
      phase -= 2.0f * M_PI;
    sound **s = &next_sound;
    if (!next_sound) {
      global_arena.used = 0;
    }
    while (*s) {
      sound *ss = *s;
      if (ss->time++ > ss->duration) {
        *s = ss->next;
        continue;
      }
      block_base[i] +=
          ss->volume * sinf(2.0f * M_PI * ss->freq * ss->time / SAMPLE_RATE);
      s = &(*s)->next;
    }
  }
  sample += block_size;
  return block_base;
}

EMSCRIPTEN_KEEPALIVE
int get_sample() { return sample; }

EMSCRIPTEN_KEEPALIVE
void setup() {
  global_arena.used = 0;
  global_arena.capacity = 1000;
  global_arena.memory = global_arena_memory;
}

EMSCRIPTEN_KEEPALIVE
int play_sound(float freq) {
  sound *s = arena_push(&global_arena, sound);
  s->duration = 5000;
  s->freq = freq;
  s->time = 0;
  s->next = next_sound;
  s->volume = 1;
  next_sound = s;
  return next_sound->freq;
}

EMSCRIPTEN_KEEPALIVE
int count_sounds() {
  sound *s = next_sound;
  int i = 0;
  while (s) {
    i++;
    s = s->next;
  }
  return i;
}
