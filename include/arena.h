#ifndef ARENA_H_
#define ARENA_H_
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** START HEADER SECTION */

#define M_arena_module(module_label) \
  ArenaModule module_label;          \
  arena_module_init(&module_label);

typedef struct ArenaModule ArenaModule;
typedef struct Arena Arena;

struct Arena {
  void *data;
  size_t cursor;
  size_t capacity;
  Arena *next;
};

struct ArenaModule {
  Arena *(*init_arena)(size_t);
  void (*deinit_arena)(Arena *);
  void *(*arena_alloc)(Arena *, size_t);
#if defined(PROJECT_TEST_BUILD)
  size_t (*get_alloc_count)(void);
  size_t (*get_free_count)(void);
#endif // PROJECT_TEST_BUILD
};

void arena_module_init(ArenaModule *self);

/** END HEADER SECTION */

/** START IMPL SECTION */

#ifdef ARENA_IMPLEMENTATION

#define M_handle_alloc_failed(ptr)                  \
  do {                                              \
    if (!(ptr)) {                                   \
      fprintf(stderr, "[FATAL] allocation failed"); \
      exit(1);                                      \
    }                                               \
  } while (0)

#if defined(PROJECT_TEST_BUILD)
static uint8_t alloc_count = 0;
static uint8_t free_count = 0;
static size_t get_alloc_count(void)
{
  return alloc_count;
}
static size_t get_free_count(void)
{
  return free_count;
}
#endif

static void *alloc(size_t size)
{
#if defined(PROJECT_TEST_BUILD)
  alloc_count++;
#endif // PROJECT_TEST_BUILD
  return malloc(size);
}

static void free_mem(void* ptr)
{
#if defined(PROJECT_TEST_BUILD)
  free_count++;
#endif // PROJECT_TEST_BUILD
  free(ptr);
}

static Arena *init_arena(size_t arena_size)
{
  Arena *arena = alloc(sizeof(Arena));
  M_handle_alloc_failed(arena);
  memset(arena, 0, sizeof(Arena));

  void *data = alloc(arena_size);
  M_handle_alloc_failed(data);
  memset(data, 0, arena_size);

  arena->capacity = arena_size;
  arena->cursor = 0;
  arena->data = data;

  return arena;
}

static void deinit_arena(Arena *self)
{
  Arena *arena_iter = self;
  while (arena_iter != NULL) {
    Arena *arena_next = arena_iter->next;
    free_mem(arena_iter->data);
    free_mem(arena_iter);
    arena_iter = arena_next;
  }
}

static void *arena_alloc(Arena *self, size_t size)
{
  if (size > self->capacity) {
    fprintf(stderr,
            "[FATAL] requested size for allocation greater than or equal to configured capacity.\n");
    exit(1);
  }
  Arena *effective_arena = self;
  while ((effective_arena->cursor + size) > effective_arena->capacity) {
    if (effective_arena->next)
      effective_arena = effective_arena->next;
    else {
      /** allocate another arena */
      Arena *next = init_arena(effective_arena->capacity);
      effective_arena->next = next;
      effective_arena = next;
      break;
    }
  }
  /** cast first to uint8_t to make the void* aligned to bytes, then cast back to void* for genericity */
  void *location_of_requested_memory =
          (void *)((uint8_t *)effective_arena->data + effective_arena->cursor);
  /** increment the cursor to the next available section of memory */
  effective_arena->cursor += size;

  return location_of_requested_memory;
}

void arena_module_init(ArenaModule *self)
{
  self->init_arena = &init_arena;
  self->deinit_arena = &deinit_arena;
  self->arena_alloc = &arena_alloc;
#if defined(PROJECT_TEST_BUILD)
  self->get_alloc_count = &get_alloc_count;
  self->get_free_count = &get_free_count;
#endif // PROJECT_TEST_BUILD
}

#endif /** ARENA_IMPLEMENTATION */

/** END IMPL SECTION */

#endif // !ARENA_H_
