#include "../include/arena.h"

int main(void)
{
  M_arena_module(Arenas);

  Arena *arena = Arenas.init_arena(10);

  /* @TODO [ ] currently these are just malloc and free.  */
  /*            make updates in ../include/arena.h        */

  Arenas.arena_alloc(arena, 1);
  Arenas.deinit_arena(arena);

  return 0;
}
