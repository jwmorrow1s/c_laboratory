#include "../include/arena.h"
#include <stdint.h>
#include <stdio.h>

int main(void)
{
  M_arena_module(Arenas);

  Arena *arena = Arenas.init_arena(3);

  /* @TODO [ ] currently these are just malloc and free.  */
  /*            make updates in ../include/arena.h        */

  uint8_t *a = Arenas.arena_alloc(arena, 2);
  a[0] = 'a';
  a[1] = '\0';
  printf("a: %s\n", a);

  uint8_t *b = Arenas.arena_alloc(arena, 2);

  b[0] = 'b';
  b[1] = '\0';
  printf("b: %s\n", b);

  Arenas.deinit_arena(arena);

  return 0;
}
