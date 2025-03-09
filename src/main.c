#include "../include/arena.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define M_expect(cond, test_text)                            \
  do {                                                       \
    printf("%s#L%d %s: " test_text "\n", __FILE__, __LINE__, \
           (cond) ? "PASS" : (test_failures++, "FAIL"));     \
  } while (0)
#define M_declare_test_start(test)              \
  do {                                          \
    printf("BEGIN\t\t" test " test\n");         \
    printf("------------------------------\n"); \
  } while (0)
#define M_declare_test_end(test)                \
  do {                                          \
    printf("------------------------------\n"); \
    printf("END\t\t" test " test\n\n");           \
  } while (0)

uint8_t test_failures = 0;

int main(void)
{
  /** for everything #include'd, ignore something so it doesn't flag a warning */
  (void)printf;
  uint8_t _a = 0;
  (void)_a;
  (void)arena_module_init;
  (void)malloc;
  /**  */

#ifdef TEST_ARENA
  M_declare_test_start("arena");

  M_arena_module(Arenas);

  Arena *arena = Arenas.init_arena(3);
  M_expect(arena->capacity == 3,
           "newly initialized arena has expected capacity");
  M_expect(arena->cursor == 0, "newly initialized arena has expected cursor");
  M_expect(arena->data != NULL, "newly initialized arena has non null data");
  M_expect(arena->next == NULL, "expect next arena to be null upon initialization");

  uint8_t *a = Arenas.arena_alloc(arena, 2);
  M_expect(a != NULL,
           "pointer of allocation of first arena of size 2 is not null");
  M_expect(arena->capacity == 3,
           "allocated memory from arena has same capacity");
  M_expect(arena->cursor == 2, "allocated memory from arena has updated cursor");
  M_expect(arena->next == NULL, "expect next arena to be null until initial arena's capacity is exceeded");

  uint8_t *a1 = Arenas.arena_alloc(arena, 1);
  (void) a1;
  M_expect(arena->cursor == 3, "additional small allocation from arena has cursor == capacity");

  uint8_t *b = Arenas.arena_alloc(arena, 2);
  M_expect(b != NULL,
           "pointer of allocation of second arena of size 2 is not null");
  M_expect(arena->cursor == 3, "expect previous arena's cursor to be unchanged when another is allocated");
  M_expect(arena->capacity == 3, "expect previous arena's capacity to be unchanged when another is allocated");
  M_expect(arena->next->cursor == 2, "expect cursor to go to 2 on the next arena");
  M_expect(arena->next->capacity == 3, "expect capacity to stay the same as the previous arena's");
  M_expect(arena->next->next == NULL, "expect there to only be two arenas until second arena's capacity is exceeded");

  uint8_t *c = Arenas.arena_alloc(arena, 2);
  M_expect(arena->cursor == 3, "expect original arena's cursor to be unchanged when another is allocated");
  M_expect(arena->capacity == 3, "expect original arena's capacity to be unchanged when another is allocated");
  M_expect(arena->next->cursor == 2, "expect previous arena's cursor to be the same");
  M_expect(arena->next->capacity == 3, "expect previous arena's capacity be the same");
  M_expect(arena->next->next->next == NULL, "expect final arena's next ptr to be null");
  M_expect(arena->next->next->cursor == 2, "expect final arena's cursor to be correct");
  M_expect(arena->next->next->capacity == 3, "expect final arena's cursor to be correct");
  M_expect(c != NULL,
           "pointer of allocation of third arena of size 2 is not null");
  /** @TODO implement reset arenas. Implement macro testing allocs and frees are correct */

  Arenas.deinit_arena(arena);

  M_declare_test_end("arena");

/** undef redefined labels */
#undef expect
#undef malloc
#undef free
/** /undef */
#endif /* ifdef TEST_ARENA */

  if(test_failures){
    printf("[ATTN] There are test failures!\n");
    return 1;
  }

  return 0;
}
