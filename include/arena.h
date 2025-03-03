#ifndef ARENA_H_
#define ARENA_H_
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/** START HEADER SECTION */

#define M_arena_module(module_label) \
  ArenaModule module_label; \
	arena_module_init(&module_label);

typedef struct ArenaModule ArenaModule;
typedef struct Arena Arena;

struct Arena {
	uintptr_t* data; 
	size_t cursor; 
	size_t capacity; 
	Arena* next;
};

struct ArenaModule {
	Arena* (*init_arena)(size_t);
	void (*deinit_arena)(Arena*);
	void* (*arena_alloc)(Arena*, size_t);
};

void arena_module_init(ArenaModule *self);

/** END HEADER SECTION */

/*---------------------------------------------------------------------------*/

/** START IMPL SECTION */

#ifdef ARENA_IMPLEMENTATION
#define M_handle_alloc_failed(ptr) \
  do { \
		if(!(ptr)){ \
			fprintf(stderr, "[FATAL] allocation failed"); \
			exit(1);  \
		} \
  } while(0)

static Arena* init_arena(size_t arena_size){
	Arena* arena = malloc(sizeof(Arena));
	M_handle_alloc_failed(arena);
	arena->capacity = arena_size;
	arena->cursor = 0;
	return arena;
}

static void deinit_arena(Arena* self){
	free(self->data);
	free(self);
}

static void* arena_alloc(Arena* self, size_t size){
	self->data = malloc(size);
	M_handle_alloc_failed(self->data);
	return self->data;
}

void arena_module_init(ArenaModule *self){
	self->init_arena = &init_arena;
	self->deinit_arena = &deinit_arena;
	self->arena_alloc = &arena_alloc;
}

#endif /** ARENA_IMPLEMENTATION */

/** END IMPL SECTION */

/*---------------------------------------------------------------------------*/

#endif // !ARENA_H_
