#include "./allocator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_START_RED "\033[31m"
#define COLOR_END "\033[0m"
#define M_print_out_of_memory()                                              \
  fprintf(stderr,                                                            \
          COLOR_START_RED "[FATAL] " COLOR_END "%s.%d#%s :: Out of Memory!", \
          __FILE_NAME__, __LINE__, __func__)

static void *standard_allocator_alloc(Allocator *self, size_t size)
{
  (void)self;
  return malloc(size);
}

static void standard_allocator_free(Allocator *self, void *bytes)
{
  (void)self;
  free(bytes);
}

static void *static_allocator_alloc(Allocator *self, size_t size)
{
  StaticAllocatorContext *context = (StaticAllocatorContext *)self->context;

  if (context->_cursor + size > context->_capacity) {
    M_print_out_of_memory();
    exit(1);
  }

  context->_cursor += size;

  return &((uintptr_t *)context->_data)[context->_cursor];
}

/** noop */
static void static_allocator_free(Allocator *self, void *bytes)
{
  (void)self;
  (void)bytes;
}

static ArenaAllocatorContext *arena_context_create(size_t arena_size)
{
  size_t context_size = sizeof(ArenaAllocatorContext);
  ArenaAllocatorContext *context = malloc(context_size);

  if (!context) {
    M_print_out_of_memory();
    exit(1);
  }
  memset(context, 0, context_size);

  context->_data = malloc(arena_size);
  if (!context->_data) {
    /** error */
    M_print_out_of_memory();
    exit(1);
  }
  memset(context->_data, 0, arena_size);

  context->_capacity = arena_size;
  context->_cursor = 0;

  return context;
}

static void *arena_allocator_alloc(Allocator *self, size_t size)
{
  /** TODOs
   * what to do when amount asked > capacity?
   *	some reasonable limit for allocation - constant
   */
  ArenaAllocatorContext *context = (ArenaAllocatorContext *)self->context;

  if (context->_cursor + size > context->_capacity) {
    ArenaAllocatorContext *next = context->_next;

    if (!next) {
      /** alloc new arena */
      next = arena_context_create(context->_capacity);
    }

    /** copy cursor before increment */
    size_t cursor_before_update = next->_cursor;
    /** increment the cursor by the requested size */
    next->_cursor += size;

    return &((uintptr_t *)next->_data)[cursor_before_update];
  }

  /** copy cursor before increment */
  size_t cursor_before_update = context->_cursor;
  /** increment the cursor by the requested size */
  context->_cursor += size;

  return &((uintptr_t *)context->_data)[cursor_before_update];
}

/** noop */
static void arena_allocator_free(Allocator *self, void *bytes)
{
  (void)self;
  (void)bytes;
}

static void static_allocator_context_init(StaticAllocatorContext *context,
                                          size_t initial_size, void *data)
{
  context->_cursor = 0;
  context->_data = data;
  context->_capacity = initial_size;
}

void standard_allocator_init(Allocator *self)
{
  self->context = NULL;
  self->free = &standard_allocator_free;
  self->alloc = &standard_allocator_alloc;
}

void standard_allocator_deinit(Allocator *self)
{
  (void)self;
}

void static_allocator_init(Allocator *self, StaticAllocatorContext *context,
                           size_t initial_size, void *data)
{
  static_allocator_context_init(context, initial_size, data);

  self->context = context;
  self->free = &static_allocator_free;
  self->alloc = &static_allocator_alloc;
}

void static_allocator_deinit(Allocator *self)
{
  static_allocator_context_init(self->context, 0, NULL);
}

void arena_allocator_init(Allocator *self, size_t arena_size)
{
  self->context = arena_context_create(arena_size);
  self->free = arena_allocator_free;
  self->alloc = arena_allocator_alloc;
}

void arena_allocator_deinit(Allocator *self)
{
  ArenaAllocatorContext *current = self->context;
  ArenaAllocatorContext *next = NULL;

  while (current != NULL) {
    /* store a pointer to next (temporarily) */
    next = current->_next;

    /* free the current data and current pointer itself */
    free(current->_data);
    free(current);

    /* point current to next */
    current = next;
  }
}
