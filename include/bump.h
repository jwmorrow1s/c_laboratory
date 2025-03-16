#ifndef BUMP_H_
#define BUMP_H_
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mem_err.h"

typedef struct BumpModule BumpModule;
typedef struct Bump Bump;

struct Bump {
  void *_data;
  size_t _cursor;
  size_t _capacity;
};

struct BumpModule {
  AllocationResult (*bump_init)(Bump *, void *, size_t);
  AllocationResult (*alloc)(Bump *, size_t, void **);
  AllocationResult (*reset)(Bump *);
};

extern BumpModule ModuleBump;

#endif // !BUMP_H_

#ifdef BUMP_IMPLEMENTATION

static AllocationResult bump_init(Bump *self, void *data, size_t size)
{
  memset(data, 0, size);
  self->_data = data;
  self->_cursor = 0;
  self->_capacity = size;

  return MemoryErrors.NoError;
}

/// returns NULL on error
static AllocationResult bump_alloc(Bump *self, size_t size, void **out)
{
  if (self->_cursor + size > self->_capacity) {
    fprintf(stderr, "[FATAL] out of memory\n");
    return MemoryErrors.MaximumCapacityExceeded;
  }

  /** increment the cursor to the next available section of memory */
  *out = (void *)((uint8_t *)self->_data + self->_cursor);

  self->_cursor += size;

  return MemoryErrors.NoError;
}

static AllocationResult bump_reset(Bump *self)
{
  self->_cursor = 0;
  self->_capacity = self->_capacity;

  return MemoryErrors.NoError;
}

BumpModule ModuleBump = (BumpModule){
  .bump_init = &bump_init,
  .alloc = &bump_alloc,
  .reset = &bump_reset,
};

#endif // !BUMP_IMPLEMENTATION
