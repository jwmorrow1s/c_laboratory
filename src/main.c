#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#if defined(PROJECT_TEST_BUILD)
#include <stdlib.h>
#include "../include/arena.h"
#endif

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
    printf("END\t\t" test " test\n\n");         \
  } while (0)

static uint8_t test_failures = 0;

enum {
  TEST_ARENA = 1,
  TEST_ALL,
};

static uint8_t is_test(uint8_t check, uint8_t test_type)
{
  return check & test_type;
}

static uint8_t eql(const char *a, char *b)
{
  uint8_t iter = 0;
  while (a[iter] == b[iter] && a[iter] != '\0')
    iter++;
  return a[iter] == b[iter];
}

int main(int argc, char **argv)
{
#if defined(PROJECT_TEST_BUILD)

  uint8_t tests = 0;

  if (argc == 1)
    tests = TEST_ALL;
  for (int i = 1; i < argc; i++) {
    char *value = argv[i];
    if (eql("arena", value))
      tests |= TEST_ARENA;
  }

  if (tests == TEST_ALL || is_test(tests, TEST_ARENA)) {
    M_declare_test_start("arena");

    M_arena_module(Arenas);

    Arena *arena = Arenas.init_arena(3);
    M_expect(Arenas.get_free_count() == 0, "expect no calls to free when module initialized");
    M_expect(Arenas.get_alloc_count() == 2, "expect 2 calls to alloc for every arena initialization");
    M_expect(arena->capacity == 3 && arena->cursor == 0 && arena->data != NULL && arena->next == NULL,
             "expect initialized arena values");

    uint8_t *a = Arenas.arena_alloc(arena, 2);
    M_expect(a != NULL,
             "pointer of allocation of first arena of size 2 is not null");
    M_expect(arena->capacity == 3 && arena->cursor == 2 && arena->next == NULL,
             "expected values for arena-based allocation of size 2");

    Arenas.arena_alloc(arena, 1);

    M_expect(arena->cursor == 3,
             "additional small allocation from arena has cursor == capacity");

    uint8_t *b = Arenas.arena_alloc(arena, 2);
    M_expect(b != NULL,
             "pointer of allocation of second arena of size 2 is not null");
    M_expect(
            arena->cursor == 3 && arena->capacity == 3,
            "expect previous arena's state to be unchanged after another allocation exceeds original's capacity");
    M_expect(arena->next->cursor == 2 && arena->next->capacity == 3 && arena->next->next == NULL,
             "expect next arena's state after another allocation of size 2");
    M_expect(Arenas.get_alloc_count() == 4, "expect 4 calls to alloc after another arena initialized");

    uint8_t *c = Arenas.arena_alloc(arena, 2);
    M_expect(c != NULL,
             "pointer of allocation of third arena of size 2 is not null");
    M_expect(
            arena->cursor == 3 && arena->capacity == 3,
            "expect original arena's cursor to still be unchanged after yet another arena added");
    M_expect(arena->next->cursor == 2 && arena->next->capacity == 3,
             "expect previous arena's state to be unchanged after allocation of another arena");
    M_expect(arena->next->next->next == NULL && arena->next->next->cursor == 2 && arena->next->next->capacity == 3,
             "expect final arena's state");
    M_expect(Arenas.get_alloc_count() == 6, "expect 6 calls to alloc after final arena initialized");

    Arenas.deinit_arena(arena);

    M_expect(Arenas.get_alloc_count() == Arenas.get_free_count(),
             "expect call to free for every call to alloc");

    M_declare_test_end("arena");
  }

  if (test_failures) {
    printf("[ATTN] There are test failures!\n");
    return 1;
  }

  return 0;
#else
  printf("[INFO] Not a test build\n");
  return 0;
#endif // PROJECT_TEST_BUILD
}
