#include "../include/allocator.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
	Allocator allocator;

	arena_allocator_init(&allocator, 13);

	char *allocated_bytes = M_alloc(&allocator, 13);
	memcpy(allocated_bytes, "Hello World!", 13);
	printf("%s\n", allocated_bytes);

	M_free(&allocator, allocated_bytes);

	arena_allocator_deinit(&allocator);
	return 0;
}
