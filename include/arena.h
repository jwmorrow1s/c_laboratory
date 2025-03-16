#ifndef ARENA_H_
#define ARENA_H_
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem_err.h"

typedef struct ArenaModule ArenaModule;
typedef struct Arena Arena;

struct Arena {
  void *_data;
  size_t _cursor;
  size_t _capacity;
  Arena *_next;
};

struct ArenaModule {
  AllocationResult (*arena_init)(size_t, Arena **);
  AllocationResult (*arena_deinit)(Arena *);
  AllocationResult (*alloc)(Arena *, size_t, void **);
  AllocationResult (*reset)(Arena *);
#if defined(PROJECT_TEST_BUILD)
  size_t (*get_alloc_count)(void);
  size_t (*get_free_count)(void);
#endif // PROJECT_TEST_BUILD
};

extern ArenaModule ModuleArena;

#endif // !ARENA_H_

#ifdef ARENA_IMPLEMENTATION

#define M_handle_alloc_failed(ptr)                  \
  do {                                              \
    if (!(ptr)) {                                   \
      fprintf(stderr, "[FATAL] allocation failed"); \
      return MemoryErrors.AllocationFailed;         \
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
#endif // PROJECT_TEST_BUILD

static void *alloc(size_t size)
{
#if defined(PROJECT_TEST_BUILD)
  alloc_count++;
#endif // PROJECT_TEST_BUILD
  return malloc(size);
}

static void free_mem(void *ptr)
{
#if defined(PROJECT_TEST_BUILD)
  free_count++;
#endif // PROJECT_TEST_BUILD
  free(ptr);
}

static AllocationResult arena_init(size_t arena_size, Arena **out)
{
  Arena *arena = alloc(sizeof(Arena));
  M_handle_alloc_failed(arena);
  memset(arena, 0, sizeof(Arena));

  void *data = alloc(arena_size);
  M_handle_alloc_failed(data);
  memset(data, 0, arena_size);

  arena->_capacity = arena_size;
  arena->_cursor = 0;
  arena->_data = data;

  *out = arena;

  return MemoryErrors.NoError;
}

static AllocationResult arena_deinit(Arena *self)
{
  Arena *arena_iter = self;
  while (arena_iter != NULL) {
    Arena *arena_next = arena_iter->_next;
    free_mem(arena_iter->_data);
    free_mem(arena_iter);
    arena_iter = arena_next;
  }

  return MemoryErrors.NoError;
}

static AllocationResult arena_reset(Arena *self)
{
  for (Arena *current = self; current != NULL; current = current->_next) {
    current->_cursor = 0;
  }

  return MemoryErrors.NoError;
}

static AllocationResult arena_alloc(Arena *self, size_t size, void **out)
{
  if (size > self->_capacity) {
    fprintf(stderr,
            "[FATAL] requested size for allocation greater than or equal to configured capacity.\n");
    exit(1);
  }
  Arena *effective_arena = self;
  while ((effective_arena->_cursor + size) > effective_arena->_capacity) {
    if (effective_arena->_next)
      effective_arena = effective_arena->_next;
    else {
      /** allocate another arena */
      Arena *next = NULL;
      AllocationResult next_result =
              arena_init(effective_arena->_capacity, &next);

      if (next_result != MemoryErrors.NoError) {
        return next_result;
      }

      effective_arena->_next = next;
      effective_arena = next;
      break;
    }
  }
  /** cast first to uint8_t to make the void* aligned to bytes, then cast back to void* for genericity */
  *out = (void *)((uint8_t *)effective_arena->_data + effective_arena->_cursor);
  /** increment the cursor to the next available section of memory */
  effective_arena->_cursor += size;

  return MemoryErrors.NoError;
}

ArenaModule ModuleArena = (ArenaModule){
  .arena_init = &arena_init,
  .arena_deinit = &arena_deinit,
  .alloc = &arena_alloc,
  .reset = &arena_reset,
#if defined(PROJECT_TEST_BUILD)
  .get_alloc_count = &get_alloc_count,
  .get_free_count = &get_free_count,
#endif // PROJECT_TEST_BUILD
};

#endif // ARENA_IMPLEMENTATION
