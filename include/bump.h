#ifndef BUMP_H_
#define BUMP_H_
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mem_err.h"

#define M_bump_module(module_label) \
  BumpModule module_label;          \
  bump_module_init(&module_label);

typedef struct BumpModule BumpModule;
typedef struct Bump Bump;

struct Bump {
  void *data;
  size_t cursor;
  size_t capacity;
};

struct BumpModule {
  AllocationResult (*bump_init)(Bump *, void *, size_t);
  AllocationResult (*bump_alloc)(Bump *, size_t, void **);
  AllocationResult (*bump_reset)(Bump *);
};

void bump_module_init(BumpModule *self);

#endif // !BUMP_H_

#ifdef BUMP_IMPLEMENTATION

static MemoryErrorModule MemoryErrors;
static uint8_t is_mem_err_module_init = 0;

static void local_mem_err_module_init(void)
{
  if (!is_mem_err_module_init) {
    is_mem_err_module_init = 1;
    mem_err_module_init(&MemoryErrors);
  }
}

static AllocationResult bump_init(Bump *self, void *data, size_t size)
{
  local_mem_err_module_init();

  memset(data, 0, size);
  self->data = data;
  self->cursor = 0;
  self->capacity = size;

  return MemoryErrors.NoError;
}

/// returns NULL on error
static AllocationResult bump_alloc(Bump *self, size_t size, void **out)
{
  local_mem_err_module_init();

  if (self->cursor + size > self->capacity) {
    fprintf(stderr, "[FATAL] out of memory\n");
    return MemoryErrors.MaximumCapacityExceeded;
  }

  /** increment the cursor to the next available section of memory */
  *out = (void *)((uint8_t *)self->data + self->cursor);

  self->cursor += size;

  return MemoryErrors.NoError;
}

static AllocationResult bump_reset(Bump *self)
{
  local_mem_err_module_init();

  self->cursor = 0;
  self->capacity = self->capacity;

  return MemoryErrors.NoError;
}

void bump_module_init(BumpModule *self)
{
  self->bump_init = &bump_init;
  self->bump_alloc = &bump_alloc;
  self->bump_reset = &bump_reset;
}

#endif // !BUMP_IMPLEMENTATION
