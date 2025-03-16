#ifndef STANDARD_ALLOC_H
#define STANDARD_ALLOC_H
#include <stddef.h>
#include <stdint.h>
#include "mem_err.h"
#include "allocator.h"
/** @TODO redefine module style here. Entire File is incorrect at the moment */

#define M_standard_allocator_module(module_label) \
  StandardAllocatorModule module_label;          \
  standard_alloc_module_init(&module_label);

typedef struct StandardAllocatorModule StandardAllocatorModule;

struct StandardAllocatorModule {
  void (*init)(Allocator*);
};

void standard_allocator_module_init(AllocatorModule *self);

#endif // !STANDARD_ALLOC_H


#ifdef STANDARD_ALLOC_IMPLEMENTATION

static AllocatorModule Allocators;
static uint8_t is_allocator_module_init = 0;
static MemoryErrorModule MemoryErrors;
static uint8_t is_mem_err_module_init = 0;

static void local_allocator_module_init(void)
{
  if (!is_allocator_module_init) {
    is_allocator_module_init = 1;
    allocator_module_init(&AllocModule);
  }
}

static void local_mem_err_module_init(void)
{
  if (!is_mem_err_module_init) {
    is_mem_err_module_init = 1;
    mem_err_module_init(&MemoryErrors);
  }
}

static AllocationResult standard_alloc(Allocator *_self, size_t size, void **out){
  *out = malloc(size);
  if (!out) {                                   
    fprintf(stderr, "[FATAL] allocation failed"); 
    return MemoryErrors.AllocationFailed;         
  }                                               
  return MemoryErrors.NoError;
}
static AllocationResult standard_free(Allocator *_self, void* ptr){
  (void)_self;
  free(ptr);
  return MemoryErrors.NoError;
}

static void standard_allocator_init(Allocator* self){
  local_allocator_module_init();
  local_mem_err_module_init();

  Allocators.allocator_init(self, NULL, &standard_alloc, &standard_free);
}

void standard_allocator_module_init(AllocatorModule *self){
  self->init = &standard_allocator_init;
}

#endif // STANDARD_ALLOC_IMPLEMENTATION
