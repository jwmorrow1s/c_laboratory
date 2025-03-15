#ifndef ARENA_H_
#define ARENA_H_
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mem_err.h"

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
  AllocationResult (*arena_init)(size_t, Arena **);
  AllocationResult (*arena_deinit)(Arena *);
  AllocationResult (*arena_alloc)(Arena *, size_t, void **);
  AllocationResult (*arena_reset)(Arena *);
#if defined(PROJECT_TEST_BUILD)
  size_t (*get_alloc_count)(void);
  size_t (*get_free_count)(void);
#endif // PROJECT_TEST_BUILD
};

void arena_module_init(ArenaModule *self);

#endif // !ARENA_H_

#ifdef ARENA_IMPLEMENTATION

static MemoryErrorModule MemoryErrors;
static uint8_t is_mem_err_module_init = 0;

static void local_mem_err_module_init(void)
{
  if (!is_mem_err_module_init) {
    is_mem_err_module_init = 1;
    mem_err_module_init(&MemoryErrors);
  }
}

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
#endif

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
  local_mem_err_module_init();
  Arena *arena = alloc(sizeof(Arena));
  M_handle_alloc_failed(arena);
  memset(arena, 0, sizeof(Arena));

  void *data = alloc(arena_size);
  M_handle_alloc_failed(data);
  memset(data, 0, arena_size);

  arena->capacity = arena_size;
  arena->cursor = 0;
  arena->data = data;

  *out = arena;

  return MemoryErrors.NoError;
}

static AllocationResult arena_deinit(Arena *self)
{
  local_mem_err_module_init();
  Arena *arena_iter = self;
  while (arena_iter != NULL) {
    Arena *arena_next = arena_iter->next;
    free_mem(arena_iter->data);
    free_mem(arena_iter);
    arena_iter = arena_next;
  }

  return MemoryErrors.NoError;
}

static AllocationResult arena_reset(Arena *self)
{
  local_mem_err_module_init();
  for (Arena *current = self; current != NULL; current = current->next) {
    current->cursor = 0;
  }

  return MemoryErrors.NoError;
}

static AllocationResult arena_alloc(Arena *self, size_t size, void **out)
{
  local_mem_err_module_init();
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
      Arena *next = NULL;
      AllocationResult next_result = arena_init(effective_arena->capacity, &next);

      if(next_result != MemoryErrors.NoError){
        return next_result;
      }

      effective_arena->next = next;
      effective_arena = next;
      break;
    }
  }
  /** cast first to uint8_t to make the void* aligned to bytes, then cast back to void* for genericity */
  *out = (void *)((uint8_t *)effective_arena->data + effective_arena->cursor);
  /** increment the cursor to the next available section of memory */
  effective_arena->cursor += size;

  return MemoryErrors.NoError;
}

void arena_module_init(ArenaModule *self)
{
  self->arena_init = &arena_init;
  self->arena_deinit = &arena_deinit;
  self->arena_alloc = &arena_alloc;
  self->arena_reset = &arena_reset;
#if defined(PROJECT_TEST_BUILD)
  self->get_alloc_count = &get_alloc_count;
  self->get_free_count = &get_free_count;
#endif // PROJECT_TEST_BUILD
}

#endif // !ARENA_IMPLEMENTATION
