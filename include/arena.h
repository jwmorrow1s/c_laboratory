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
};

void arena_module_init(ArenaModule *self);

/** END HEADER SECTION */

/*---------------------------------------------------------------------------*/

/** START IMPL SECTION */

#ifdef ARENA_IMPLEMENTATION

#define M_handle_alloc_failed(ptr)                  \
  do {                                              \
    if (!(ptr)) {                                   \
      fprintf(stderr, "[FATAL] allocation failed"); \
      exit(1);                                      \
    }                                               \
  } while (0)

static Arena *init_arena(size_t arena_size)
{
  Arena *arena = malloc(sizeof(Arena));
  M_handle_alloc_failed(arena);
  memset(arena, 0, sizeof(Arena));

  void *data = malloc(arena_size);
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
    free(arena_iter->data);
    free(arena_iter);
    arena_iter = arena_next;
  }
}

static void *arena_alloc(Arena *self, size_t size)
{
  if (size >= self->capacity) {
    fprintf(stderr,
            "[FATAL] requested size for allocation greater than or equal to configured capacity.\n");
    exit(1);
  }
  Arena *effective_arena = self;
  while ((effective_arena->cursor + size) >= effective_arena->capacity) {
    /** mark current arena as used up */
    effective_arena->cursor = effective_arena->capacity;
    /** allocate another arena */
    Arena *next = init_arena(size);
    /** assign new arena to next ptr */
    effective_arena->next = next;
    /** assign the iterator to the newly alloc'd arena */
    effective_arena = effective_arena->next;
  }
  /** increment the cursor of the effective arena at this point */
  effective_arena->cursor += size;
  /** cast first to uint8_t to make the void* aligned to bytes, then cast back to void* for genericity */
  return (void *)((uint8_t *)effective_arena->data + effective_arena->cursor);
}

void arena_module_init(ArenaModule *self)
{
  self->init_arena = &init_arena;
  self->deinit_arena = &deinit_arena;
  self->arena_alloc = &arena_alloc;
}

#endif /** ARENA_IMPLEMENTATION */

/** END IMPL SECTION */

/*---------------------------------------------------------------------------*/

#endif // !ARENA_H_
