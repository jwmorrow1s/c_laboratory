#ifndef ALLOCATOR_H
#define ALLOCATOR_H
#include <stddef.h>

/** 
  The following two macros are for dynamic dispatch of a general  
    Allocator interface of alloc & free  
    There's a void *context which dictates behavior for these fns 
*/
#define M_alloc(ALLOCATOR, size) \
  (ALLOCATOR)->alloc((ALLOCATOR), (size))
#define M_free(ALLOCATOR, bytes) \
  (ALLOCATOR)->free((ALLOCATOR), (bytes))

typedef struct Allocator Allocator;
typedef struct StaticAllocatorContext StaticAllocatorContext;
typedef struct ArenaAllocatorContext ArenaAllocatorContext;

struct ArenaAllocatorContext {
	void *_data;
	size_t _capacity;
	size_t _cursor;
	ArenaAllocatorContext *_next;
};

struct StaticAllocatorContext {
	void *_data;
	size_t _capacity;
	size_t _cursor;
};

typedef void* (*AllocFn)(Allocator* self, size_t size);
typedef void  (*FreeFn)(Allocator* self, void* obj);

struct Allocator {
	void*   context;
	AllocFn alloc;
	FreeFn  free;
};

void standard_allocator_init(Allocator*);
void standard_allocator_deinit(Allocator*);

void static_allocator_init(Allocator*, StaticAllocatorContext*, size_t, void *data);
void static_allocator_deinit(Allocator*);

void arena_allocator_init(Allocator*, size_t);
void arena_allocator_deinit(Allocator*);
#endif // !ALLOCATOR_H
