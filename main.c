#include <stddef.h>
#define MAX_SOUNDS 20
#define SAMPLE_RATE 44100
#define SOUND_BUFFER_SIZE 4410
#define M_PI 3.14159265358979323846f

float sinf(float x) {
  double sign = 1;
  if (x < 0) {
    sign = -1.0;
    x = -x;
  }

  while (x < -2 * M_PI)
    x += 2 * M_PI;
  while (x > 2 * M_PI)
    x -= 2 * M_PI;
  double res = 0;
  double term = x;
  int k = 1;
  while (res + term != res) {
    res += term;
    k += 2;
    term *= -x * x / k / (k - 1);
  }

  return sign * res;
}

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

unsigned char global_arena_memory[sizeof(sound) * MAX_SOUNDS];
struct arena global_arena;

typedef struct sound_list {
  struct sound *next_sound;
  struct sound *free_sound;
} sound_list;
struct sound_list playing = {0, 0};
float phase = 0.0f;

float sound_buffer[SOUND_BUFFER_SIZE] = {};
unsigned int sound_buffer_index = 0;

float envelop(int Progress, int Duration) {
  int Attack = Duration / 4;
  int Decay = 2 * Duration / 4;
  int Sustain = 3 * Duration / 4;
  if (Progress < Attack) {
    return (float)Progress / (float)Attack;
  }
  if (Progress < Decay) {
    return 1.0f - 0.5f * (float)(Progress - Attack) / (float)(Decay - Attack);
  }
  if (Progress < Sustain) {
    return 0.5f;
  }
  return 0.5f -
         0.5f * (float)(Progress - Sustain) / (float)(Duration - Sustain);
}

float *generate_block(float freq, int block_size) {

  if (sound_buffer_index + block_size >= SOUND_BUFFER_SIZE) {
    sound_buffer_index = 0;
  }
  const float step = 2.0f * M_PI * freq / SAMPLE_RATE;
  float *block_base = &sound_buffer[sound_buffer_index];

  for (int i = 0; i < block_size; i++) {

    block_base[i] = sinf(phase);
    phase += step;
    if (phase > 2.0f * M_PI) {
      phase -= 2.0f * M_PI;
    }
    sound **s = &playing.next_sound;

    int count = 1;
    while (*s) {
      sound *ss = *s;
      if (ss->time > ss->duration) {
        *s = ss->next;
        ss->next = playing.free_sound;
        playing.free_sound = ss;
        continue;
      }
      if (ss->time >= 0) {
        count++;
        block_base[i] += ss->volume * envelop(ss->time, ss->duration) *
                         sinf(2.0f * M_PI * ss->freq * ss->time / SAMPLE_RATE) /
                         2.0;
      }
      ss->time++;
      s = &(*s)->next;
    }
  }

  sound_buffer_index += block_size;

  return block_base;
}

float *get_sound_buffer_base() { return sound_buffer; }

int get_sound_buffer_size() { return SOUND_BUFFER_SIZE; }

void setup() {
  global_arena.used = 0;
  global_arena.capacity = sizeof(global_arena_memory);
  global_arena.memory = global_arena_memory;
}

int play_sound(float freq, float duration) {

  sound *s = playing.free_sound;
  if (!s) {
    s = arena_push(&global_arena, sound);
  } else {
    playing.free_sound = s->next;
  }
  if (!s) {
    return 0;
  }
  s->duration = (int)(SAMPLE_RATE * duration);
  s->freq = freq;
  s->time = 0;
  s->next = playing.next_sound;
  s->volume = 1.0;
  playing.next_sound = s;
  return 1;
}

int count_playing_sounds() {
  sound *s = playing.next_sound;
  int i = 0;
  while (s) {
    i++;
    s = s->next;
  }
  return i;
}

int count_free_sounds() {
  sound *s = playing.free_sound;
  int i = 0;
  while (s) {
    i++;
    s = s->next;
  }
  return i;
}
