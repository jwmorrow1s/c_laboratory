#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_
#endif // !ALLOCATOR_H_
#include <stddef.h>
#include "mem_err.h"
 
#define M_Allocator_module(module_label) \
  AllocatorModule module_label;          \
  allocator_module_init(&module_label);

typedef struct AllocatorModule AllocatorModule;
typedef struct Allocator Allocator;

struct AllocatorModule {
  void (*allocator_init)(Allocator*, void*, AllocationResult (*alloc)(Allocator *, size_t, void **), AllocationResult (*free)(void *));
};

struct Allocator {
  void *context;
  AllocationResult (*alloc)(Allocator*, size_t, void**);
  AllocationResult (*free)(void *);
};

void allocator_module_init(AllocatorModule *self);

#ifdef ALLOCATOR_IMPLEMENTATION

static void allocator_init(Allocator* self, void* context, AllocationResult (*alloc)(Allocator *, size_t, void **), AllocationResult (*free)(void *)) {
  self->context = context;
  self->alloc = alloc;
  self->free = free;
}

void allocator_module_init(AllocatorModule *self){
  self->allocator_init = &allocator_init;
}

#endif // ALLOCATOR_IMPLEMENTATION
